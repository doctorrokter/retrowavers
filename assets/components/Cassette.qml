import bb.cascades 1.4
import chachkouski.util 1.0

Container {
    id: root
    
    property string cover: "asset:///images/cover.jpg"
    property bool playing: false
    
    margin.topOffset: ui.du(10)
    preferredHeight: ui.du(35)
    preferredWidth: ui.du(55)
    horizontalAlignment: HorizontalAlignment.Center
    
    layout: DockLayout {}
    
    ImageView {
        imageSource: root.cover
        preferredHeight: ui.du(31)
        preferredWidth: ui.du(49)
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
            maxWidth: ui.du(7)
            maxHeight: ui.du(7)
        
            margin.topOffset: ui.du(11.5)
            margin.leftOffset: ui.du(12.2)
        }
        
        ImageView {
            id: rightWheel
            imageSource: "asset:///images/cogwheel.png"
            maxWidth: ui.du(7)
            maxHeight: ui.du(7)
        
            margin.topOffset: ui.du(11.5)
            margin.leftOffset: ui.du(35.6)
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
}