import bb.cascades 1.4
import chachkouski.util 1.0

Container {
    id: root
    
    property string cover: "asset:///images/cover.jpg"
    property bool playing: false
    
    property int screenWidth: 1440
    property int screenHeight: 1440
    
    property double cogwheelBigSize: 8.9
    property double cogwheelSmallSize: 5.5
    property double cogwheelStandardSize: 7
    
    property double cogwheelBigTopOffset: 14.8
    property double cogwheelSmallOffset: 9.8
    property double cogwheelStandardOffset: 11.5
    
    margin.topOffset: ui.du(10)
    
    preferredHeight: {
        if (deviceIsSmall()) {
            return ui.du(29);
        } else if (deviceIsBig()) {
            return ui.du(45);
        }
        return ui.du(35);
    }
    
    preferredWidth: {
        if (deviceIsSmall()) {
            return ui.du(48);
        } else if (deviceIsBig()) {
            return ui.du(70);
        }
        return ui.du(55);
    }
    
    layout: DockLayout {}
    
    ImageView {
        id: cassetteCover
        imageSource: root.cover
        
        preferredHeight: {
            if (deviceIsSmall()) {
                return parent.preferredHeight - ui.du(3.5);
            } else if (deviceIsBig()) {
                return parent.preferredHeight - ui.du(5);
            }
            return parent.preferredHeight - ui.du(3.9);
        }
        preferredWidth: {
            if (deviceIsSmall()) {
                return parent.preferredWidth - ui.du(1);
            } else if (deviceIsBig()) {
                return parent.preferredWidth - ui.du(1);
            }
            return parent.preferredWidth - ui.du(5);
        }
        horizontalAlignment: HorizontalAlignment.Center
        scalingMethod: ScalingMethod.AspectFill
    }
    
    Container {
        layout: DockLayout {}
        
        horizontalAlignment: HorizontalAlignment.Center
        preferredHeight: parent.preferredHeight
        preferredWidth: parent.preferredWidth
        
        ImageView {
            imageSource: "asset:///images/cassette-body.png"
            preferredHeight: parent.preferredHeight
            preferredWidth: parent.preferredWidth
        }
        
        ImageView {
            id: leftWheel
            imageSource: "asset:///images/cogwheel.png"
            
            maxWidth: {
                if (deviceIsSmall()) {
                    return ui.du(root.cogwheelSmallSize);
                } else if (deviceIsBig()) {
                    return ui.du(root.cogwheelBigSize);
                }
                return ui.du(root.cogwheelStandardSize);
            }
            
            maxHeight: {
                if (deviceIsSmall()) {
                    return ui.du(root.cogwheelSmallSize);
                } else if (deviceIsBig()) {
                    return ui.du(root.cogwheelBigSize);
                }
                return ui.du(root.cogwheelStandardSize);
            }
            
            margin.topOffset: {
                if (deviceIsSmall()) {
                    return ui.du(root.cogwheelSmallOffset);
                } else if (deviceIsBig()) {
                    return ui.du(root.cogwheelBigTopOffset);
                }
                return ui.du(root.cogwheelStandardOffset);
            }
            
            margin.leftOffset: {
                if (deviceIsSmall()) {
                    return ui.du(11);
                } else if (deviceIsBig()) {
                    return ui.du(15.5);
                }
                return ui.du(12.2);
            }
        }
        
        ImageView {
            id: rightWheel
            imageSource: "asset:///images/cogwheel.png"
            
            maxWidth: {
                if (deviceIsSmall()) {
                    return ui.du(root.cogwheelSmallSize);
                } else if (deviceIsBig()) {
                    return ui.du(root.cogwheelBigSize);
                }
                return ui.du(root.cogwheelStandardSize);
            }
            
            maxHeight: {
                if (deviceIsSmall()) {
                    return ui.du(root.cogwheelSmallSize);
                } else if (deviceIsBig()) {
                    return ui.du(root.cogwheelBigSize);
                }
                return ui.du(root.cogwheelStandardSize);
            }
            
            margin.topOffset: {
                if (deviceIsSmall()) {
                    return ui.du(root.cogwheelSmallOffset);
                } else if (deviceIsBig()) {
                    return ui.du(root.cogwheelBigTopOffset);
                }
                return ui.du(root.cogwheelStandardOffset);
            }
            
            margin.leftOffset: {
                if (deviceIsSmall()) {
                    return ui.du(31.5);
                } else if (deviceIsBig()) {
                    return ui.du(45.4);
                }
                return ui.du(35.6);
            }
        }
    }
    
    attachedObjects: [
        Timer {
            id: leftWheelTimer
            
            interval: 50
            
            onTimeout: {
                leftWheel.rotationZ -= 1;
            }
        },
        
        Timer {
            id: rightWheelTimer
            
            interval: 25
            
            onTimeout: {
                rightWheel.rotationZ -= 1;
            }
        }
    ]
    
    onPlayingChanged: {
        if (playing) {
            leftWheelTimer.start();
            rightWheelTimer.start();
        } else {
            leftWheelTimer.stop();
            rightWheelTimer.stop();
        }
    }
    
    function deviceIsSmall() {
        return root.screenWidth === 720 && root.screenHeight === 720;
    }
    
    function deviceIsBig() {
        return root.screenWidth === 1440 && root.screenHeight === 1440;
    }
}