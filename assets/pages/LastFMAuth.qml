import bb.cascades 1.4
import bb.system 1.2
import "../style"

Page {
    id: root
    
    property bool success: true
    property bool signedIn: false
    property string signedInUser: "user"
    
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
                opacity: 0.3
            }
            
            Container {
                id: signOut
                    
                verticalAlignment: VerticalAlignment.Center
                horizontalAlignment: HorizontalAlignment.Center
                
                maxWidth: ui.du(75)
                
                visible: root.signedIn
                
                Label {
                    text: "Signed in as"
                    textStyle.base: textStyle.style
                    textStyle.fontSize: FontSize.Large
                    horizontalAlignment: HorizontalAlignment.Center
                }
                
                Label {
                    text: root.signedInUser
                    textStyle.base: textStyle.style
                    textStyle.fontSize: FontSize.Large
                    textStyle.color: ui.palette.primaryDark
                    horizontalAlignment: HorizontalAlignment.Center
                }
                
                Button {
                    margin.topOffset: ui.du(7)
                    horizontalAlignment: HorizontalAlignment.Fill
                    text: qsTr("Sign out") + Retranslate.onLocaleOrLanguageChanged
                    color: ui.palette.primaryDark
                    
                    onClicked: {
                        _appConfig.set("lastfm_key", "");
                        _appConfig.set("lastfm_name", "");
                    }
                }
            }
            
            Container {
                id: signIn
                
                visible: !root.signedIn
                
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
        _appConfig.settingsChanged.connect(root.updateSettings);
        updateSettings();
    }
    
    function cleanUp() {
        _lastFM.authenticationFinished.disconnect(root.onAuth);
        _appConfig.settingsChanged.disconnect(root.updateSettings);
    }
    
    function updateSettings() {
        var lastFMKey = _appConfig.get("lastfm_key");
        var lastFMName = _appConfig.get("lastfm_name");
        root.signedIn = lastFMKey !== undefined && lastFMKey !== "";
        root.signedInUser = lastFMName;
    }
    
    function onAuth(message, success) {
        spinner.stop();
        root.success = success;
        if (success) {
            toast.body = message;
        } else {
            toast.body = qsTr("Error login. Check your credentials.") + Retranslate.onLocaleOrLanguageChanged
        }
        
        toast.show();
    }
}
