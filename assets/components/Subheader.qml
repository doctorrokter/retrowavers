import bb.cascades 1.4

Container {
    id: root

    signal option1Selected();
    signal option2Selected();
    signal option3Selected();

    property bool isShown: true
    property string option1: "Playlist"
    property string option2: "Radio"
    property string option3: "Favourite"
    property int height: rootLUH.layoutFrame.height
    property bool option1Enabled: true
    property bool option2Enabled: true
    property bool option3Enabled: true

    // Which tab is active. Settable from outside so the app can open on the source
    // the user last listened to without pretending they just tapped the tab.
    property alias selectedIndex: segmented.selectedIndex

    horizontalAlignment: HorizontalAlignment.Fill
    verticalAlignment: VerticalAlignment.Top

    Container {
        horizontalAlignment: HorizontalAlignment.Fill
        SegmentedControl {
            id: segmented

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
                },

                Option {
                    text: option3
                    enabled: root.option3Enabled
                }
            ]

            onSelectedIndexChanged: {
                root.isShown = true;
                switch (selectedIndex) {
                    case 0: option1Selected(); break;
                    case 1: option2Selected(); break;
                    case 2: option3Selected(); break;
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
