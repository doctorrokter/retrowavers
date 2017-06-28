import bb.cascades 1.4
import bb.multimedia 1.4

Container {
    
    id: root
    
    property bool playing: false
    property string cover: "asset:///images/cover.jpg"
    property string title: ""
    property string currentTime: ""
    property string duration: ""
    
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
        playing: root.playing
        
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
        TextStyleDefinition {
            id: textStyle
            fontFamily: "Newtown"
            rules: [
                FontFaceRule {
                    source: "asset:///fonts/NEWTOW_I.ttf"
                    fontFamily: "Newtown"
                }
            ]
        },
        
        MediaPlayer {
            id: player
            
            function nextAfterLoad() {
                _tracksController.next();
                _api.loaded.disconnect(player.nextAfterLoad);
            }
            
            onPlaybackCompleted: {
                var result = _tracksController.next();       
                if (!result) {
                    _api.loaded.connect(player.nextAfterLoad);
                    _api.load();
                }  
            }
            
            onPositionChanged: {
                root.currentTime = getMediaTime(position);
            }
        }
        
//        NowPlayingConnection {
//            id: nowplaying
//            
//            connectionName: "Retrowavers"
//            
//            onAcquired: {
//                var track = _tracksService.active;
//                duration = track.duration;
//                position = 0;
//                mediaState = MediaState.Started;
//                nowplaying.setMetaData({"artist": track.title});
//            }
//            
//            onPause: {
//                player.pause();
//            }
//            
//            onPlay: {
//                player.play();
//            }
//            
//            onRevoked: {
//                player.stop();
//            }
//        }
    ]
    
    function getMediaTime(time) {
        var seconds = Math.round(time / 1000);
        var h = Math.floor(seconds / 3600) < 10 ? '0' + Math.floor(seconds / 3600) : Math.floor(seconds / 3600);
        var m = Math.floor((seconds / 60) - (h * 60)) < 10 ? '0' + Math.floor((seconds / 60) - (h * 60)) : Math.floor((seconds / 60) - (h * 60));
        var s = Math.floor(seconds - (m * 60) - (h * 3600)) < 10 ? '0' + Math.floor(seconds - (m * 60) - (h * 3600)) : Math.floor(seconds - (m * 60) - (h * 3600));
        return m + ':' + s;
    }
    
    onCreationCompleted: {
        _tracksController.played.connect(function(track) {
            root.playing = true;
            root.title = track.title;
            root.currentTime = "00:00";
            root.duration = getMediaTime(track.duration);
            root.cover = track.imagePath;
//            nowplaying.iconUrl = track.imagePath;
//            nowplaying.acquire();
            player.sourceUrl = track.streamUrl;
            player.play();
        });
        _tracksController.paused.connect(function() {
            root.playing = false;
            player.pause();
        });
    }    
}