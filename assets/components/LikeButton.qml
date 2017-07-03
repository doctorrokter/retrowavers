import bb.cascades 1.4

Container {
    id: root
    
    signal like();
    
    property bool favourite: false
    property int screenWidth: 720
    property int screenHeight: 720
    
    property string filled: "asset:///images/heart_filled.png"
    property string empty: "asset:///images/heart_empty.png"
    
    ImageView {
        id: heart
        
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
            }
        ]
    }
    
    onFavouriteChanged: {
        if (favourite) {
            heart.imageSource = root.filled;
            heart.filterColor = ui.palette.primaryDark;
        } else {
            heart.imageSource = root.empty;
            heart.filterColor = Color.White;
        }
    }
    
    function deviceIsSmall() {
        return root.screenWidth === 720 && root.screenHeight === 720;
    }
}