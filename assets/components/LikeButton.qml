import bb.cascades 1.4
import "../style"

Container {
    id: root
    
    signal like();
    
    property bool favourite: false
    property int screenWidth: 1440
    property int screenHeight: 1440
    property int percentage: 55
    
    property string filled: "asset:///images/heart_filled.png"
    property string empty: "asset:///images/heart_empty.png"
    
    layout: DockLayout {}
    
    ImageView {
        id: heart
        horizontalAlignment: HorizontalAlignment.Center
        
        filterColor: {
            if (root.favourite) {
                return ui.palette.primaryDark;
            }
            return Color.White;
        }
        
        imageSource: {
            if (root.favourite) {
                return root.filled;
            }
            return root.empty;
        }
        
        maxWidth: {
            if (deviceIsSmall()) {
                return ui.du(8);
            }
            return ui.du(10);
        }
        maxHeight: {
            if (deviceIsSmall()) {
                return ui.du(8);
            }
            return ui.du(10);
        }
        
        gestureHandlers: [
            DoubleTapHandler {
                onDoubleTapped: {
                    scaleHeartUp.play();
                }
            }
        ]
        
        shortcuts: [
            Shortcut {
                key: "l"
                
                onTriggered: {
                    scaleHeartUp.play();
                }
            }
        ]
        
        attachedObjects: [
            ScaleTransition {
                id: scaleHeartUp
                
                duration: 250
                
                fromX: 1
                fromY: 1
                
                toX: 0
                toY: 0
                
                onEnded: {
                    heart.imageSource = "asset:///images/heart_filled.png";
                    heart.filterColor = ui.palette.primaryDark;
                    scaleHeartDown.play();
                }
            },
            
            ScaleTransition {
                id: scaleHeartDown
                
                duration: 250
                
                fromX: 0
                fromY: 0
                
                toX: 1
                toY: 1
                
                onEnded: {
                    root.like();
                }
            },
            
            ScaleTransition {
                id: scaleHeartUpAfterDownload
                
                duration: 250
                
                fromX: 0.8
                fromY: 0.8
                
                toX: 1.1
                toY: 1.1
                
                onEnded: {
                    scaleHeartDownAfterDownload.play();
                }
            },
            
            ScaleTransition {
                id: scaleHeartDownAfterDownload
                
                duration: 250
                
                fromX: 1.1
                fromY: 1.1
                
                toX: 1.0
                toY: 1.0
            }
        ]
    }
    
    Label {
        text: root.percentage
        horizontalAlignment: HorizontalAlignment.Center
        
        visible: root.percentage > 0 && root.percentage < 100
        
        textStyle.base: textStyle.style
        textStyle.fontSize: {
            if (deviceIsSmall()) {
                return FontSize.XXSmall;
            } if (deviceIsBig()) {
                return FontSize.Medium;
            }
            return FontSize.Small;
        }
        
        margin.topOffset: {
            if (deviceIsSmall()) {
                return ui.du(1.5);
            } else if (deviceIsBig()) {
                return ui.du(2);
            }
            return ui.du(1.8);
        }
    }
    
    attachedObjects: [
        RetroTextStyleDefinition {
            id: textStyle
        }
    ]
    
    onFavouriteChanged: {
        if (favourite) {
            heart.imageSource = root.filled;
            heart.filterColor = ui.palette.primaryDark;
        } else {
            heart.imageSource = root.empty;
            heart.filterColor = Color.White;
        }
    }
    
    onPercentageChanged: {
        if (percentage === 100) {
            scaleHeartUpAfterDownload.play();
        }
    }
    
    function deviceIsSmall() {
        return root.screenWidth === 720 && root.screenHeight === 720;
    }
    
    function deviceIsBig() {
        return root.screenWidth === 1440 && root.screenHeight === 1440;
    }
}