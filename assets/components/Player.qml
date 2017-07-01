import bb.cascades 1.4
import bb.multimedia 1.4
import "../style"

Container {
    
    id: root
    
    property bool playing: false
    property string trackId: ""
    property string cover: "asset:///images/cover.jpg"
    property string title: ""
    property string currentTime: ""
    property string duration: ""
    
    property bool asleep: false
    property bool scrobbled: false
    property int startTime: 0
    property int durationMillis: 0
    property string artistName: ""
    property string trackName: ""
    property bool scrobblerEnabled: false
    
    property int fourMinutes: 60000 * 4
    
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
        
        verticalAlignment: VerticalAlignment.Center
        horizontalAlignment: HorizontalAlignment.Fill
    }
    
    Container {
        verticalAlignment: VerticalAlignment.Center
        horizontalAlignment: HorizontalAlignment.Fill
        PlayerBottom {
            id: playerBottom
            
            playing: root.playing
            margin.bottomOffset: ui.du(3.5)
            
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
                _tracksController.next();
                _api.loaded.disconnect(playerBottom.nextAfterLoad);
            }
        }
    }
    
    Container {
        horizontalAlignment: HorizontalAlignment.Center
        verticalAlignment: VerticalAlignment.Bottom
        
        margin.bottomOffset: ui.du(2)
        Container {
            margin.topOffset: ui.du(5)
            horizontalAlignment: HorizontalAlignment.Center
            Label {
                text: root.title
                textStyle.base: textStyle.style
                textStyle.fontSize: FontSize.XLarge
                textFormat: TextFormat.Html
                multiline: true
            }    
        }
        
        Container {
            margin.topOffset: ui.du(2.5)
            horizontalAlignment: HorizontalAlignment.Center
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            
            Label {
                text: root.currentTime
                textStyle.base: textStyle.style
                textStyle.fontSize: FontSize.Medium
            }
            
            Label {
                text: "/"
                visible: root.currentTime !== ""
                textStyle.base: textStyle.style
                textStyle.fontSize: FontSize.Medium
            }
            
            Label {
                text: root.duration
                textStyle.base: textStyle.style
                textStyle.fontSize: FontSize.Medium
                textStyle.color: ui.palette.primary
            }
        }
    }
    
    attachedObjects: [
        RetroTextStyleDefinition {
            id: textStyle
        },
        
        MediaPlayer {
            id: player
            
            function nextAfterLoad() {
                _tracksController.next();
                _api.loaded.disconnect(player.nextAfterLoad);
            }
            
            onMediaStateChanged: {
                if (mediaState === MediaState.Started) {
                    if (root.scrobblerEnabled) {
                        _lastFM.track.updateNowPlaying(root.artistName, root.trackName);
                    }
                }
            }
            
            onPlaybackCompleted: {
                root.next();  
            }
            
            onPositionChanged: {
                if (root.playing && !root.asleep) {
                    root.currentTime = getMediaTime(position);
                }
                
                if (root.scrobblerEnabled) {
                    if (!root.scrobbled) {
                        if (position >= (root.durationMillis / 2) || position >= root.fourMinutes) {
                            root.scrobbled = true;
                            _lastFM.track.scrobble(root.artistName, root.trackName, root.startTime);
                        }
                    }
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
                nowplaying.setMetaData({"artist": root.artistName, "track": root.trackName});
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
                player.stop();
            }
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
            player.sourceUrl = track.streamUrl;
            root.scrobbled = false;
            root.startTime = new Date().getTime() / 1000;
            root.durationMillis = track.duration;
            
            var parts = getArtistAndTrack(track.title);
            root.artistName = parts.artist;
            root.trackName = parts.track;
        }
        if (player.mediaState === MediaState.Paused) {
            nowplaying.play();
        } else {
            nowplaying.acquire();
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
        root.cover = track.imagePath;
    }
    
    function getArtistAndTrack(title) {
        var track = _tracksService.active.toMap();
        var parts = track.title.split(" – ");
        return {artist: parts[0].trim(), track: parts[1].trim()};
    }
    
    function updateSettings() {
        var lastFMKey = _appConfig.get("lastfm_key");
        root.scrobblerEnabled = lastFMKey !== undefined && lastFMKey !== "";
    }
    
    onCreationCompleted: {
        _tracksController.played.connect(root.play);
        Application.asleep.connect(root.stopRendering);
        Application.awake.connect(root.resumeRendering);
        _appConfig.settingsChanged.connect(root.updateSettings);
        updateSettings();
    }    
}