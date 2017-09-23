import bb.cascades 1.4

Page {
    id: root
    
    property string type: "vk"
    
    titleBar: TitleBar {
        title: qsTr("Share") + Retranslate.onLocaleOrLanguageChanged
    }
    
    ScrollView {
        scrollRole: ScrollRole.Main
        
        horizontalAlignment: HorizontalAlignment.Fill
        
        Container {
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Fill
            
            layout: DockLayout {}
            
            leftPadding: ui.du(2)
            rightPadding: ui.du(2)
            topPadding: ui.du(2)
            
            attachedObjects: [
                LayoutUpdateHandler {
                    id: mainLUH
                }
            ]
            
            Container {
                ImageView {
                    id: image
                    horizontalAlignment: HorizontalAlignment.Fill
                    maxHeight: mainLUH.layoutFrame.height / 2;
                    
                    imageSource: _tracksService.active.imagePath
                    scalingMethod: ScalingMethod.AspectFill
                }
                
                Label {
                    text: qsTr("Message") + Retranslate.onLocaleOrLanguageChanged
                }
                
                TextArea {
                    id: message
                    text: qsTr("Now listening in Retrowavers: The Legacy app on my BlackBerry 10 smartphone") + Retranslate.onLocaleOrLanguageChanged
                }
                
                Button {
                    horizontalAlignment: HorizontalAlignment.Center
                    text: qsTr("Share!") + Retranslate.onLocaleOrLanguageChanged
                    imageSource: {
                        if (root.type === "vk") {
                            return "asset:///images/ic_vk.png";
                        } else if (root.type === "fb") {
                            return "asset:///images/ic_facebook.png";
                        }
                    }
                    
                    onClicked: {
                        spinner.start();
                        switch (root.type) {
                            case "vk": 
                                var track = _tracksService.active.toMap();
                                _vkController.share(track, message.text);
                                break;
                            case "fb": 
                                var track = _tracksService.active.toMap();
                                _fbController.share(track, message.text);
                                break;
                        }
                    }
                }
            }
            
            ActivityIndicator {
                id: spinner
                verticalAlignment: VerticalAlignment.Center
                horizontalAlignment: HorizontalAlignment.Center
                minWidth: ui.du(20)
            }
        }
    }
    
    onCreationCompleted: {
        var track = _tracksService.active.toMap();
        message.text = track.title + "\n\n" + message.text;
    }
}
