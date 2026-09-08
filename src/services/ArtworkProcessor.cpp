/*
 * ArtworkProcessor.cpp - see ArtworkProcessor.hpp.
 */

#include "ArtworkProcessor.hpp"
#include "../Common.hpp"

#include <QtGui/QImage>
#include <QtGui/QImageWriter>
#include <QtConcurrentRun>
#include <QFuture>
#include <QVector>
#include <QDir>
#include <QFile>
#include <QDebug>

// Cover: big enough for the cassette on a 1440x1440 Passport, small enough that
// decoding it on every layout is free. Blur: rendered tiny on purpose - the
// ImageView upscales it with AspectFill (~4-5x), which multiplies the softness
// for free, and blur cost scales with pixel count.
static const int kCoverPx = 720;
static const int kWorkPx = 160;
static const int kRadius = 22;

// JPEG keeps a cover around 100 KB where PNG would take ~800 KB. The plugin is
// normally there (the API serves .jpeg artwork we already decode), but if this
// build cannot WRITE jpeg we fall back to PNG rather than produce nothing.
static bool jpegSupported() {
    static int cached = -1;
    if (cached < 0) {
        cached = QImageWriter::supportedImageFormats().contains("jpg") ? 1 : 0;
        qDebug() << "===>>> ArtworkProcessor: jpeg writing supported: " << (cached == 1) << endl;
    }
    return cached == 1;
}

static QString baseNameFor(const QString& artworkUrl) {
    // ".../artwork/<uuid>.png" -> "<uuid>"
    QString filename = artworkUrl.split("/").last();
    int dot = filename.lastIndexOf(".");
    return dot > 0 ? filename.left(dot) : filename;
}

ArtworkProcessor::ArtworkProcessor(QObject* parent) : QObject(parent) {}

ArtworkProcessor::~ArtworkProcessor() {}

QString ArtworkProcessor::coverPathFor(const QString& artworkUrl) {
    return QDir::currentPath() + IMAGES + "/" + baseNameFor(artworkUrl)
            + "_c" + QString::number(kCoverPx) + (jpegSupported() ? ".jpg" : ".png");
}

QString ArtworkProcessor::blurPathFor(const QString& artworkUrl) {
    // The blur parameters live in the filename: changing them regenerates the
    // cache instead of silently serving images rendered with the old settings.
    return QDir::currentPath() + IMAGES + "/" + baseNameFor(artworkUrl)
            + "_b" + QString::number(kWorkPx) + "_" + QString::number(kRadius) + ".png";
}

void ArtworkProcessor::process(const QString& trackId, const QString& sourcePath, const QString& artworkUrl) {
    QString coverPath = coverPathFor(artworkUrl);
    QString blurPath = blurPathFor(artworkUrl);

    if (QFile::exists(coverPath) && QFile::exists(blurPath)) {
        emit ready(trackId, coverPath, blurPath);
        return;
    }

    if (m_inFlight.contains(sourcePath)) {
        return;   // already rendering; the result reaches every listener
    }
    m_inFlight.insert(sourcePath);

    QFutureWatcher<QStringList>* watcher = new QFutureWatcher<QStringList>(this);
    m_pendingTrack.insert(watcher, trackId);
    m_pendingSource.insert(watcher, sourcePath);
    QObject::connect(watcher, SIGNAL(finished()), this, SLOT(onFinished()));
    watcher->setFuture(QtConcurrent::run(&ArtworkProcessor::render, sourcePath, coverPath, blurPath));
}

void ArtworkProcessor::onFinished() {
    QFutureWatcher<QStringList>* watcher = static_cast<QFutureWatcher<QStringList>*>(sender());
    if (watcher == NULL) {
        return;
    }

    QString trackId = m_pendingTrack.take(watcher);
    QString sourcePath = m_pendingSource.take(watcher);
    QStringList result = watcher->result();
    m_inFlight.remove(sourcePath);
    watcher->deleteLater();

    if (result.size() == 2) {
        emit ready(trackId, result.at(0), result.at(1));
    } else {
        qDebug() << "===>>> ArtworkProcessor: render failed for " << sourcePath << endl;
        emit ready(trackId, QString(""), QString(""));
    }
}

// --- StackBlur (Mario Klingemann), ported from BBTelega ----------------------
// A fast Gaussian-like blur (the one Telegram Desktop/Android and CSS blur
// filters use): a triangular-weighted window [ (r+1) - |j| ] over [-r, r] slid
// across each line, kept O(len) per line by a small "stack" plus running in/out
// sums. Normalised by dividing by the window weight (r+1)^2 instead of the
// classic mul/shr lookup tables - same result, and no 255-entry constant tables
// to mis-transcribe (they are only a fixed-point speed hack, pointless on a
// 160px working image). Runs on downscaled R/G/B int planes; edges are clamped
// (replicated). in and out must be distinct.
static void stackBlurLine(const int* in, int* out, int len, int stride, int r) {
    if (len <= 0) {
        return;
    }
    if (r < 1) {
        for (int i = 0; i < len; ++i) {
            out[i * stride] = in[i * stride];   // no blur -> straight copy
        }
        return;
    }

    const int div = 2 * r + 1;
    const long divsum = static_cast<long>(r + 1) * (r + 1);   // sum of the triangular weights
    QVector<int> stack(div);

    long sum = 0;      // sum of weight*value over the current window
    long sumIn = 0;    // sum of the values on the incoming (leading) side
    long sumOut = 0;   // sum of the values on the outgoing (trailing) side
    for (int i = -r; i <= r; ++i) {
        const int idx = i < 0 ? 0 : (i >= len ? len - 1 : i);   // clamp to the edge
        const int val = in[idx * stride];
        stack[i + r] = val;
        const int rank = r + 1 - (i < 0 ? -i : i);
        sum += static_cast<long>(val) * rank;
        if (i > 0) {
            sumIn += val;
        } else {
            sumOut += val;
        }
    }

    int sp = r;   // stack pointer, at the window centre
    for (int x = 0; x < len; ++x) {
        out[x * stride] = static_cast<int>(sum / divsum);
        sum -= sumOut;

        int stackStart = sp - r + div;          // element leaving the trailing edge (circular)
        if (stackStart >= div) {
            stackStart -= div;
        }
        sumOut -= stack[stackStart];

        int idx = x + r + 1;                     // next pixel entering the leading edge
        if (idx > len - 1) {
            idx = len - 1;
        }
        const int val = in[idx * stride];
        stack[stackStart] = val;
        sumIn += val;
        sum += sumIn;

        sp++;
        if (sp >= div) {
            sp -= div;
        }
        const int val2 = stack[sp];              // the new centre moves from the in-side to the out-side
        sumOut += val2;
        sumIn -= val2;
    }
}

// One full 2-D StackBlur of a plane: horizontal (plane -> tmp) then vertical
// (tmp -> plane), so the result lands back in plane. tmp is scratch of the same size.
static void stackBlurPlane(int* plane, int* tmp, int w, int h, int r) {
    for (int y = 0; y < h; ++y) {
        stackBlurLine(plane + y * w, tmp + y * w, w, 1, r);
    }
    for (int x = 0; x < w; ++x) {
        stackBlurLine(tmp + x, plane + x, h, w, r);
    }
}

QStringList ArtworkProcessor::render(const QString& sourcePath, const QString& coverPath, const QString& blurPath) {
    QStringList result;

    QImage source(sourcePath);
    if (source.isNull() || source.width() <= 0 || source.height() <= 0) {
        return result;
    }

    QImage cover = source.scaled(kCoverPx, kCoverPx, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    bool saved = jpegSupported() ? cover.save(coverPath, "JPEG", 88) : cover.save(coverPath, "PNG");
    if (!saved) {
        qDebug() << "===>>> ArtworkProcessor: cover save failed " << coverPath << endl;
        return result;
    }

    // Blur the already-downscaled cover, not the multi-megabyte original.
    QImage work = cover.scaled(kWorkPx, kWorkPx, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            .convertToFormat(QImage::Format_RGB32);
    const int w = work.width();
    const int h = work.height();
    if (w < 2 || h < 2) {
        return result;
    }

    int r = kRadius;
    const int rmax = qMax(1, qMin(w, h) / 2 - 1);
    if (r > rmax) {
        r = rmax;   // keep the window inside a small image
    }

    const int n = w * h;
    QVector<int> rp(n);
    QVector<int> gp(n);
    QVector<int> bp(n);
    QVector<int> tmp(n);
    for (int y = 0; y < h; ++y) {
        const QRgb* line = reinterpret_cast<const QRgb*>(work.constScanLine(y));
        for (int x = 0; x < w; ++x) {
            const QRgb c = line[x];
            const int i = y * w + x;
            rp[i] = qRed(c);
            gp[i] = qGreen(c);
            bp[i] = qBlue(c);
        }
    }

    stackBlurPlane(rp.data(), tmp.data(), w, h, r);   // one StackBlur (H+V) per channel
    stackBlurPlane(gp.data(), tmp.data(), w, h, r);
    stackBlurPlane(bp.data(), tmp.data(), w, h, r);

    QImage blurred(w, h, QImage::Format_RGB32);
    for (int y = 0; y < h; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(blurred.scanLine(y));
        for (int x = 0; x < w; ++x) {
            const int i = y * w + x;
            line[x] = qRgb(rp[i], gp[i], bp[i]);
        }
    }

    if (!blurred.save(blurPath, "PNG")) {
        qDebug() << "===>>> ArtworkProcessor: blur save failed " << blurPath << endl;
        return result;
    }

    result << coverPath << blurPath;
    return result;
}
