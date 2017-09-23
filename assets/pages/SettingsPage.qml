import bb.cascades 1.4
import "../components"

Page {
    id: root
    
    actionBarAutoHideBehavior: ActionBarAutoHideBehavior.HideOnScroll
    actionBarVisibility: ChromeVisibility.Overlay
    
    titleBar: TitleBar {
        title: qsTr("Settings") + Retranslate.onLocaleOrLanguageChanged
    }
    
    ScrollView {
        scrollRole: ScrollRole.Main
        
        Container {
            layout: DockLayout {}
            Container {
                horizontalAlignment: HorizontalAlignment.Fill
                verticalAlignment: VerticalAlignment.Fill
                
                Header {
                    title: qsTr("Behaviour") + Retranslate.onLocaleOrLanguageChanged
                }
                
                Container {
                    layout: DockLayout {}
                    topPadding: ui.du(2)
                    bottomPadding: ui.du(2.5)
                    leftPadding: ui.du(2.5)
                    rightPadding: ui.du(2.5)
                    horizontalAlignment: HorizontalAlignment.Fill
                    
                    Label {
                        text: qsTr("Hub notifications") + Retranslate.onLocaleOrLanguageChanged
                        verticalAlignment: VerticalAlignment.Center
                        horizontalAlignment: HorizontalAlignment.Left
                    }
                    
                    ToggleButton {
                        id: notifyToggle
                        horizontalAlignment: HorizontalAlignment.Right
                        
                        onCheckedChanged: {
                            if (checked) {
                                _appConfig.set("notify_now_playing", "true");
                            } else {
                                _appConfig.set("notify_now_playing", "false");
                            }
                        }
                    }
                }
                
                Container {
                    layout: DockLayout {}
                    topPadding: ui.du(2)
                    bottomPadding: ui.du(2.5)
                    leftPadding: ui.du(2.5)
                    rightPadding: ui.du(2.5)
                    horizontalAlignment: HorizontalAlignment.Fill
                    
                    Label {
                        text: qsTr("Last.fm scrobbling") + Retranslate.onLocaleOrLanguageChanged
                        verticalAlignment: VerticalAlignment.Center
                        horizontalAlignment: HorizontalAlignment.Left
                    }
                    
                    ToggleButton {
                        id: scrobblingToggle
                        horizontalAlignment: HorizontalAlignment.Right
                        
                        onCheckedChanged: {
                            if (checked) {
                                _appConfig.set("scrobbling", "true");
                            } else {
                                _appConfig.set("scrobbling", "false");
                            }
                        }
                    }
                }
                
                Container {
                    horizontalAlignment: HorizontalAlignment.Fill
                    minHeight: ui.du(20)
                }
            }
        }
    }
    
    function adjustNotification() {
        var notify = _appConfig.get("notify_now_playing");
        notifyToggle.checked = notify === "" || notify === "true";
    }
    
    function adjustScrobbling() {
        var scrobbling = _appConfig.get("scrobbling");
        scrobblingToggle.checked = scrobbling === "" || scrobbling === "true";
    }
    
    onCreationCompleted: {
        adjustNotification();
        adjustScrobbling();
    }
}