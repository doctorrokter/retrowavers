import bb.cascades 1.4

Container {
    id: root
    
    signal play();
    signal pause();
    signal next();
    signal prev();
    
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
                        root.prev();
                    }
                }
            ]
        }
        
        ImageView {
            id: playButton
            verticalAlignment: VerticalAlignment.Center
            horizontalAlignment: HorizontalAlignment.Center
            
            visible: !root.playing
            
            imageSource: "asset:///images/ic_play.png"
            
            gestureHandlers: [
                TapHandler {
                    onTapped: {
                        root.play();
                    }
                }
            ]                    
        }
        
        ImageView {
            id: pauseButton
            
            verticalAlignment: VerticalAlignment.Center
            horizontalAlignment: HorizontalAlignment.Center
            
            visible: root.playing
            
            imageSource: "asset:///images/ic_pause.png"
            
            gestureHandlers: [
                TapHandler {
                    onTapped: {
                        root.pause();
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
                        root.next();
                    }
                }
            ]
        }
    }
    
    attachedObjects: [
        LayoutUpdateHandler {
            id: bottomLUH
        }
    ]
}