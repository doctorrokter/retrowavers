import bb.cascades 1.4
import bb.multimedia 1.4
import bb.device 1.4
import bb.system 1.2
import chachkouski.enums 1.0
import "../style"

Container {
    
    id: root
    
    property bool playing: false
    property string trackId: ""
    property string cover: "asset:///images/cover.jpg"
    property string title: ""
    property string currentTime: ""
    property string duration: ""
    property bool favourite: false
    
    property bool asleep: false
    property int durationMillis: 0
    property string artistName: ""
    property string trackName: ""
    
    property int percentage: 0

    // Radio has no file behind it, so there is nothing to like or download.
    property bool sourceCanDownload: true
    
    property int screenWidth: 1440
    property int screenHeight: 1440
    
    horizontalAlignment: HorizontalAlignment.Fill
    verticalAlignment: VerticalAlignment.Fill
    
    layout: DockLayout {}
    
    Container {
        horizontalAlignment: HorizontalAlignment.Fill
        verticalAlignment: VerticalAlignment.Top
        
        ImageView {
            imageSource: "asset:///images/logo.png"
            horizontalAlignment: HorizontalAlignment.Center
            
            margin.topOffset: ui.du(10)
            preferredWidth: ui.du(70)
            preferredHeight: ui.du(18)
        }
    }
    
    Cassette {
        id: cassette
        cover: root.cover
        playing: root.playing && !root.asleep
        
        screenWidth: root.screenWidth
        screenHeight: root.screenHeight
        
        verticalAlignment: VerticalAlignment.Center
        horizontalAlignment: HorizontalAlignment.Fill
    }
    
    Container {
        verticalAlignment: VerticalAlignment.Center
        horizontalAlignment: HorizontalAlignment.Fill
        PlayerBottom {
            id: playerBottom
            
            playing: root.playing
            screenWidth: root.screenWidth
            screenHeight: root.screenHeight
            
            margin.bottomOffset: {
                if (deviceIsSmall()) {
                    return ui.du(0.5);
                } else if (deviceIsBig()) {
                    return ui.du(6);
                }
                return ui.du(2.5);
            }
            
            onPlay: {
                if (_tracksService.active !== null && _tracksService.active !== undefined) {
                    var tr = _tracksService.active.toMap();
                    _tracksController.play(tr);
                } else {
                    _tracksController.play({});
                }
                root.playing = true;
            }
            
            onPause: {
                root.playing = false;
                nowplaying.pause();
            }
            
            onNext: {
                root.next();
            }
            
            onPrev: {
                root.prev();
            }
            
            function nextAfterLoad() {
                _api.loaded.disconnect(playerBottom.nextAfterLoad);
                root.advanceOrWrap();
            }
        }
    }
    
    Container {
        horizontalAlignment: HorizontalAlignment.Center
        verticalAlignment: VerticalAlignment.Bottom
        
        margin.bottomOffset: ui.du(1)
                
        LikeButton {
            id: likeButton
            
            screenWidth: root.screenWidth
            screenHeight: root.screenHeight
            
            favourite: root.favourite
            percentage: root.percentage
            
            opacity: (root.sourceCanDownload && _tracksService.active !== undefined && _tracksService.active !== null) ? 1.0 : 0.0
            enabled: root.sourceCanDownload
            horizontalAlignment: HorizontalAlignment.Center
            
            onLike: {
                root.favourite = true;
                _tracksController.like();
            }
        }
        
        Container {
            margin.topOffset: ui.du(1)
            horizontalAlignment: HorizontalAlignment.Center
            Label {
                text: root.title
                textStyle.base: textStyle.style
                textFormat: TextFormat.Html
                textStyle.fontSize: {
                    if (deviceIsSmall()) {
                        return FontSize.Small;
                    } else if (deviceIsBig()) {
                        return FontSize.XLarge;
                    }
                    return FontSize.Large;
                }
                multiline: true
            }    
        }
        
        Container {
            // Live radio runs forever: no length to count towards, so the row reads
            // empty rather than 00:00 - but it keeps its space, or the title below
            // would slide down the screen.
            opacity: root.durationMillis > 0 ? 1.0 : 0.0
            
            margin.topOffset: ui.du(2.5)
            horizontalAlignment: HorizontalAlignment.Center
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            
            Label {
                text: root.currentTime
                textStyle.base: textStyle.style
                textStyle.fontSize: {
                    if (deviceIsSmall()) {
                        return FontSize.XXSmall;
                    } else if (deviceIsBig()) {
                        return FontSize.Large;
                    }
                    return FontSize.Small;
                }
            }
            
            Label {
                text: "/"
                visible: root.currentTime !== ""
                textStyle.base: textStyle.style
                textStyle.fontSize: {
                    if (deviceIsSmall()) {
                        return FontSize.XXSmall;
                    } else if (deviceIsBig()) {
                        return FontSize.Large;
                    }
                    return FontSize.Small;
                }
            }
            
            Label {
                text: root.duration
                textStyle.base: textStyle.style
                textStyle.color: ui.palette.primary
                textStyle.fontSize: {
                    if (deviceIsSmall()) {
                        return FontSize.XXSmall;
                    } else if (deviceIsBig()) {
                        return FontSize.Large;
                    }
                    return FontSize.Small;
                }
            }
        }
    }
    
    attachedObjects: [
        RetroTextStyleDefinition {
            id: textStyle
        },
        
        MediaPlayer {
            id: player
            
            equalizerPreset: getEqualizer();
            
            function nextAfterLoad() {
                _api.loaded.disconnect(player.nextAfterLoad);
                root.advanceOrWrap();
            }
            
            onError: {
                nowplaying.revoke();
                _app.toast((qsTr("Media player error: ") + Retranslate.onLocaleOrLanguageChanged) + mediaError);
            }
            
            onPlaybackCompleted: {
                root.next();  
            }
            
            onPositionChanged: {
                if (root.playing && !root.asleep) {
                    root.currentTime = getMediaTime(position);
                }
                
            }
        },
        
        NowPlayingConnection {
            id: nowplaying
            
            overlayStyle: OverlayStyle.Fancy
            duration: player.duration
            position: player.position
            mediaState: player.mediaState
            
            onAcquired: {
                var track = _tracksService.active;
                nowplaying.iconUrl = track.imagePath;
                nowplaying.setMetaData({"artist": root.artistName, "track": root.trackName, "duration": root.durationMillis, "album": ""});
                player.play();
            }
            
            onPause: {
                root.playing = false;
                player.pause();
            }
            
            onPlay: {
                root.playing = true;
                player.play();
            }
            
            onNext: {
                root.next();
            }
            
            onPrevious: {
                root.prev();
            }
            
            onRevoked: {
                root.playing = false;
                player.stop();
            }
        },
        
        DisplayInfo {
            id: display
        }
    ]
    
    function next() {
        nowplaying.revoke();
        var result = _tracksController.next();
        if (!result) {
            _api.loaded.connect(playerBottom.nextAfterLoad);
            _api.load();
        }
    }
    
    // Called once a load() finished because we had run out of tracks. If there was
    // nothing new to load, the catalogue is simply exhausted (it is ~94 tracks), so
    // start it over rather than going quiet at the end of the list.
    function advanceOrWrap() {
        if (_tracksController.next()) {
            return;
        }
        
        var tracks = _tracksService.tracks;
        if (tracks.length > 0) {
            _tracksController.play(tracks[0]);
        }
    }
    
    function prev() {
        nowplaying.revoke();
        _tracksController.prev();
    }
    
    function getMediaTime(time) {
        var seconds = Math.round(time / 1000);
        var h = Math.floor(seconds / 3600) < 10 ? '0' + Math.floor(seconds / 3600) : Math.floor(seconds / 3600);
        var m = Math.floor((seconds / 60) - (h * 60)) < 10 ? '0' + Math.floor((seconds / 60) - (h * 60)) : Math.floor((seconds / 60) - (h * 60));
        var s = Math.floor(seconds - (m * 60) - (h * 3600)) < 10 ? '0' + Math.floor(seconds - (m * 60) - (h * 3600)) : Math.floor(seconds - (m * 60) - (h * 3600));
        return m + ':' + s;
    }
    
    function play(track) {
        if (root.trackId !== track.id) {
            nowplaying.revoke();
            if (!root.asleep) {
                root.setData(track);
            }
            
            if (track.favourite && track.localPath) {
                player.sourceUrl = "file://" + track.localPath;
                console.debug("===>>> Player: play from local " + track.localPath);
            } else {
                player.sourceUrl = track.streamUrl;
            }
            
            root.durationMillis = track.duration;
            
            var parts = getArtistAndTrack(track.title);
            root.artistName = parts.artist;
            root.trackName = parts.track;
        }
        
        if (!_app.online && _tracksController.playerMode === PlayerMode.Playlist) {
            root.playing = false;
            _app.toast(qsTr("No internet connection") + Retranslate.onLocaleOrLanguageChanged);
        } else {
            if (player.mediaState === MediaState.Paused) {
                nowplaying.play();
            } else {
                nowplaying.acquire();
            }
        }
    }
    
    function pause() {
        root.playing = false;
        nowplaying.pause();
    }
    
    function stopRendering() {
        root.asleep = true;
    }
    
    function resumeRendering() {
        root.asleep = false;
        var track = _tracksService.active;
        if (track !== null && track !== undefined) {
            root.setData(track.toMap());
        }
        root.playing = nowplaying.mediaState === MediaState.Started;
    }
    
    function setData(track) {
        root.trackId = track.id;
        root.playing = true;
        root.title = track.title;
        root.currentTime = getMediaTime(player.position);
        root.duration = getMediaTime(track.duration);
        root.cover = track.imagePath !== "" ? track.imagePath : "asset:///images/cover.jpg";
        root.favourite = track.favourite;
        likeButton.percentage = 0;
    }
    
    function updateCover(id, imagePath) {
        if (root.trackId === id) {
            root.cover = "file://" + imagePath;
            nowplaying.iconUrl = "file://" + imagePath;
        }
    }
    
    function getArtistAndTrack(title) {
        var parts = title.split(" – ");
        if (parts.length < 2) {
            return {artist: "", track: title};   // a radio station, not "artist - track"
        }
        return {artist: parts[0].trim(), track: parts[1].trim()};
    }

    // A radio station moved on to the next song: the track object was patched in C++,
    // here we refresh what is on screen and in the now-playing overlay.
    function updateMetadata(id, title) {
        if (root.trackId !== id) {
            return;
        }

        root.title = title;
        var parts = getArtistAndTrack(title);
        root.artistName = parts.artist;
        root.trackName = parts.track;
        nowplaying.setMetaData({"artist": root.artistName, "track": root.trackName, "duration": root.durationMillis, "album": ""});
    }

    function updateSource() {
        root.sourceCanDownload = _api.canDownload();
    }
    
    function updateSettings() {
        
        player.equalizerPreset = getEqualizer();
    }
    
    function deviceIsSmall() {
        return root.screenWidth === 720 && root.screenHeight === 720;
    }
    
    function deviceIsBig() {
        return root.screenWidth === 1440 && root.screenHeight === 1440;
    }
    
    function downloadProgress(id, sent, total) {
        if (root.trackId === id) {
            var percentage = parseInt((sent * 100) / total);
            root.percentage = percentage;
        }
    }
    
    function onlineChanged(online) {
        if (!online && _tracksController.playerMode === PlayerMode.Playlist) {
            nowplaying.revoke();
            _app.toast(qsTr("No internet connection") + Retranslate.onLocaleOrLanguageChanged);
        }
    }
    
    function getEqualizer() {
        var equalizer = _appConfig.get("equalizer");
        if (equalizer === "") {
            return EqualizerPreset.Off;
        } else {
            return parseInt(equalizer);
        }
    }
    
    onPercentageChanged: {
        if (!root.asleep) {
            likeButton.percentage = percentage;
        }
    }
    
    onCreationCompleted: {
        root.screenWidth = display.pixelSize.width;
        root.screenHeight = display.pixelSize.height;
        
        
        _tracksController.played.connect(root.play);
        _tracksService.imageChanged.connect(root.updateCover);
        _tracksService.metadataChanged.connect(root.updateMetadata);
        _api.activeSourceChanged.connect(root.updateSource);
        updateSource();
        _tracksController.downloadProgress.connect(root.downloadProgress);
        Application.asleep.connect(root.stopRendering);
        Application.awake.connect(root.resumeRendering);
        Application.aboutToQuit.connect(function() {
            nowplaying.setMetaData({"artist": "", "track": "", "album": "", "duration": ""});
            nowplaying.revoke();
        });
        _appConfig.settingsChanged.connect(root.updateSettings);
        _app.onlineChanged.connect(root.onlineChanged);
        updateSettings();
    }    
}