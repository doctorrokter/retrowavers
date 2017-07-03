import bb.cascades 1.4

Container {
    id: root
    
    signal play();
    signal pause();
    signal next();
    signal prev();
    
    property int height: bottomLUH.layoutFrame.height
    property bool playing: false
    property int screenWidth: 1440
    property int screenHeight: 1440
    
    property double smallButtonSize: 7
    property double bigButtonSize: 10
    property double standardButtonSize: 8
    
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
            
            preferredWidth: {
                if (deviceIsSmall()) {
                    return ui.du(root.smallButtonSize);
                } else if (deviceIsBig()) {
                    return ui.du(root.bigButtonSize);
                }
                return ui.du(root.standardButtonSize);
            }
            
            preferredHeight: {
                if (deviceIsSmall()) {
                    return ui.du(root.smallButtonSize);
                } else if (deviceIsBig()) {
                    return ui.du(root.bigButtonSize);
                }
                return ui.du(root.standardButtonSize);
            }
            
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
            
            preferredWidth: {
                if (deviceIsSmall()) {
                    return ui.du(root.smallButtonSize);
                } else if (deviceIsBig()) {
                    return ui.du(root.bigButtonSize);
                }
                return ui.du(root.standardButtonSize);
            }
            
            preferredHeight: {
                if (deviceIsSmall()) {
                    return ui.du(root.smallButtonSize);
                } else if (deviceIsBig()) {
                    return ui.du(root.bigButtonSize);
                }
                return ui.du(root.standardButtonSize);
            }
            
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
            
            preferredWidth: {
                if (deviceIsSmall()) {
                    return ui.du(root.smallButtonSize);
                } else if (deviceIsBig()) {
                    return ui.du(root.bigButtonSize);
                }
                return ui.du(root.standardButtonSize);
            }
            
            preferredHeight: {
                if (deviceIsSmall()) {
                    return ui.du(root.smallButtonSize);
                } else if (deviceIsBig()) {
                    return ui.du(root.bigButtonSize);
                }
                return ui.du(root.standardButtonSize);
            } 
            
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
            
            preferredWidth: {
                if (deviceIsSmall()) {
                    return ui.du(root.smallButtonSize);
                } else if (deviceIsBig()) {
                    return ui.du(root.bigButtonSize);
                }
                return ui.du(root.standardButtonSize);
            }
            
            preferredHeight: {
                if (deviceIsSmall()) {
                    return ui.du(root.smallButtonSize);
                } else if (deviceIsBig()) {
                    return ui.du(root.bigButtonSize);
                }
                return ui.du(root.standardButtonSize);
            }
            
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
    
    function deviceIsSmall() {
        return root.screenWidth === 720 && root.screenHeight === 720;
    }
    
    function deviceIsBig() {
        return root.screenWidth === 1440 && root.screenHeight === 1440;
    }
}