import bb.cascades 1.4

Container {
    id: root   
    
    signal option1Selected();
    signal option2Selected();
    
    property bool isShown: true
    property string option1: "Playlist"
    property string option2: "Favourite"
    property int height: rootLUH.layoutFrame.height
    property bool option1Enabled: true
    property bool option2Enabled: true
             
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
                    enabled: root.option1Enabled
                },
                
                Option {
                    text: option2
                    enabled: root.option2Enabled
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