import bb.cascades 1.4
import bb.system 1.2
import "../style"

Page {
    id: root
    
    property bool success: true
    
    titleBar: TitleBar {
        kind: TitleBarKind.FreeForm
        kindProperties: FreeFormTitleBarKindProperties {
            Container {
                horizontalAlignment: HorizontalAlignment.Fill
                verticalAlignment: VerticalAlignment.Fill
                layout: DockLayout {}
                ImageView {
                    horizontalAlignment: HorizontalAlignment.Center
                    verticalAlignment: VerticalAlignment.Center
                    maxWidth: ui.du(25)
                    maxHeight: ui.du(6)
                    imageSource: "asset:///images/Lastfm_logo.png"
                }
            }
        }
    }
    
    ScrollView {
        Container {
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Fill
            
            layout: DockLayout {}
            
            ImageView {
                imageSource: "asset:///images/blur.jpg"
                horizontalAlignment: HorizontalAlignment.Fill
                verticalAlignment: VerticalAlignment.Fill
                scalingMethod: ScalingMethod.AspectFill
                opacity: 0.4
            }
            
            Container {
                verticalAlignment: VerticalAlignment.Center
                horizontalAlignment: HorizontalAlignment.Center
                
                maxWidth: ui.du(75)
                
                Label {
                    text: qsTr("Login") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: textStyle.style
                    textStyle.fontSize: FontSize.Large
                }
                
                TextField {
                    id: username
                    textStyle.base: textStyle.style
                }
                
                Label {
                    text: qsTr("Password") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: textStyle.style
                    textStyle.fontSize: FontSize.Large
                }
                
                TextField {
                    id: password
                    textStyle.base: textStyle.style
                    inputMode: TextFieldInputMode.Password
                }
                
                Button {
                    margin.topOffset: ui.du(7)
                    horizontalAlignment: HorizontalAlignment.Fill
                    text: qsTr("Sign in") + Retranslate.onLocaleOrLanguageChanged
                    color: ui.palette.primaryDark
                    
                    onClicked: {
                        spinner.start();
                        _lastFM.authenticate(username.text, password.text);
                    }
                }
            }
            
            ActivityIndicator {
                id: spinner
                verticalAlignment: VerticalAlignment.Center
                horizontalAlignment: HorizontalAlignment.Center
                minWidth: ui.du(20);
            }
            
        }
    }
    
    attachedObjects: [
        RetroTextStyleDefinition {
            id: textStyle
        },
        
        SystemToast {
            id: toast
            
            onFinished: {
                if (root.success) {
                    root.parent.pop();
                }
            }
        }
    ]
    
    onCreationCompleted: {
        _lastFM.authenticationFinished.connect(root.onAuth);
    }
    
    function cleanUp() {
        _lastFM.authenticationFinished.disconnect(root.onAuth);
    }
    
    function onAuth(message, success) {
        spinner.stop();
        root.success = success;
        toast.body = message;
        toast.show();
    }
}
