import bb.cascades 1.4
import bb.system 1.2
import bb.platform 1.3
import "../components"

Page {
    id: root
    
    titleBar: TitleBar {
        title: qsTr("About") + Retranslate.onLocaleOrLanguageChanged
    }
    
    actionBarAutoHideBehavior: ActionBarAutoHideBehavior.HideOnScroll
    actionBarVisibility: ChromeVisibility.Overlay
    
    ScrollView {
        scrollRole: ScrollRole.Main
        
        Container {
            
            leftPadding: ui.du(2.5)
            topPadding: ui.du(2.5)
            rightPadding: ui.du(2.5)
            bottomPadding: ui.du(2.5)
            
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                
                Label {
                    text: "Author:"
                    textStyle.base: textStyle.style
                }
                
                Label {
                    text: "Mikhail Chachkouski"
                    textStyle.color: ui.palette.primary
                    textStyle.fontWeight: FontWeight.W500
                    textStyle.base: textStyle.style
                    
                    gestureHandlers: [
                        TapHandler {
                            onTapped: {
                                bbwInvoke.trigger(bbwInvoke.query.invokeActionId);
                            }
                        }
                    ]
                }
                
                bottomPadding: ui.du(2.5)
            }
            
            Container {
                Label { 
                    text: "App name: " + Application.applicationName
                    textStyle.base: textStyle.style
                }
                
                Label {
                    text: "App version: " + Application.applicationVersion
                    textStyle.base: textStyle.style
                }
                
                Label {
                    text: "OS version: " + platform.osVersion
                    textStyle.base: textStyle.style
                }
            }
        }
    }
    
    attachedObjects: [
        Invocation {
            id: bbwInvoke
            query {
                uri: "appworld://vendor/97424"
                invokeActionId: "bb.action.OPEN"
                invokeTargetId: "sys.appworld"
            }
        },
        
        PlatformInfo {
            id: platform
        },
        
        TextStyleDefinition {
            id: textStyle
            fontFamily: "Newtown"
            rules: [
                FontFaceRule {
                    source: "asset:///fonts/NEWTOW_I.ttf"
                    fontFamily: "Newtown"
                }
            ]
        }
    ]
}
