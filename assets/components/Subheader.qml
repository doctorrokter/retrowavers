import bb.cascades 1.4

Container {
    id: root   
    
    signal option1Selected();
    signal option2Selected();
    
    property bool isShown: true
    property string option1: "Playlist"
    property string option2: "Favourite"
    property int height: rootLUH.layoutFrame.height
             
    horizontalAlignment: HorizontalAlignment.Fill
    verticalAlignment: VerticalAlignment.Top
    
    Container {
        horizontalAlignment: HorizontalAlignment.Fill
        SegmentedControl {
            horizontalAlignment: HorizontalAlignment.Fill
            bottomMargin: ui.du(0)
            
            options: [
                Option {
                    text: option1
                },
                
                Option {
                    text: option2
                }
            ]
            
            onSelectedIndexChanged: {
                root.isShown = true;
                if (selectedIndex === 0) {
                    option1Selected();
                } else {
                    option2Selected();
                }
            }
        }
    }
        
    attachedObjects: [
        LayoutUpdateHandler {
            id: rootLUH
        }
    ]
    
    onIsShownChanged: {
        if (isShown) {
            root.setTranslationY(0);
        } else {
            root.setTranslationY(-rootLUH.layoutFrame.height);
        }
    }
}