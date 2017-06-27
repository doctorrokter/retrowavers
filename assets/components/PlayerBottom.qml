import bb.cascades 1.4

Container {
    id: playerBottom
    
    property int height: bottomLUH.layoutFrame.height
    property bool playing: false
    
    horizontalAlignment: HorizontalAlignment.Fill
    verticalAlignment: VerticalAlignment.Bottom
    
    preferredHeight: ui.du(12)
    leftPadding: ui.du(1)
    topPadding: ui.du(1)
    rightPadding: ui.du(1)
    bottomPadding: ui.du(1)
    
    layout: StackLayout {}
    
    Container {
        horizontalAlignment: HorizontalAlignment.Fill
        
        layout: DockLayout {}
        
        ImageView {
            horizontalAlignment: HorizontalAlignment.Left
            verticalAlignment: VerticalAlignment.Center
            imageSource: "asset:///images/ic_previous.png"
            
            gestureHandlers: [
                TapHandler {
                    onTapped: {
                        _tracksController.prev();
                    }
                }
            ]
        }
        
        ImageView {
            id: playButton
            verticalAlignment: VerticalAlignment.Center
            horizontalAlignment: HorizontalAlignment.Center
            
            imageSource: {
                if (playing) {
                    return "asset:///images/ic_pause.png";
                }
                return "asset:///images/ic_play.png";
            }
            
            gestureHandlers: [
                TapHandler {
                    onTapped: {
                        playing = !playing;
                    }
                }
            ]                    
        }
        
        ImageView {
            id: nextButton
            
            horizontalAlignment: HorizontalAlignment.Right
            verticalAlignment: VerticalAlignment.Center
            imageSource: "asset:///images/ic_next.png"
            
            gestureHandlers: [
                TapHandler {
                    onTapped: {
                        var result = _tracksController.next();
                        if (!result) {
                            _api.loaded.connect(nextButton.nextAfterLoad);
                            _api.load();
                        }
                    }
                }
            ]
            
            function nextAfterLoad() {
                _tracksController.next();
                _api.loaded.disconnect(nextButton.nextAfterLoad);
            }
        }
    }
    
    attachedObjects: [
        LayoutUpdateHandler {
            id: bottomLUH
        }
    ]
    
    onPlayingChanged: {
        if (playing) {
            _tracksController.play(_tracksService.active);
        } else {
            _tracksController.pause();
        }
    }
}