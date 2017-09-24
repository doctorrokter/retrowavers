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
                    leftPadding: ui.du(2.5)
                    rightPadding: ui.du(2.5)
                    
                    DropDown {
                        id: equalizerDropDown
                        
                        title: qsTr("Equalizer preset") + Retranslate.onLocaleOrLanguageChanged
                        
                        onSelectedOptionChanged: {
                            _appConfig.set("equalizer", selectedOption.value);
                        }
                        
                        options: [
                            Option {
                                text: qsTr("Off") + Retranslate.onLocaleOrLanguageChanged
                                value: 0
                            },
                            
                            Option {
                                text: qsTr("Airplain") + Retranslate.onLocaleOrLanguageChanged
                                value: 1
                            },
                            
                            Option {
                                text: qsTr("Bass Boost") + Retranslate.onLocaleOrLanguageChanged
                                value: 2
                            },
                            
                            Option {
                                text: qsTr("Treble Boost") + Retranslate.onLocaleOrLanguageChanged
                                value: 3
                            },
                            
                            Option {
                                text: qsTr("Voice Boost") + Retranslate.onLocaleOrLanguageChanged
                                value: 4
                            },
                            
                            Option {
                                text: qsTr("Bass Lower") + Retranslate.onLocaleOrLanguageChanged
                                value: 5
                            },
                            
                            Option {
                                text: qsTr("Treble Lower") + Retranslate.onLocaleOrLanguageChanged
                                value: 6
                            },
                            
                            Option {
                                text: qsTr("Voice Lower") + Retranslate.onLocaleOrLanguageChanged
                                value: 7
                            },
                            
                            Option {
                                text: qsTr("Acoustic") + Retranslate.onLocaleOrLanguageChanged
                                value: 8
                            },
                            
                            Option {
                                text: qsTr("Dance") + Retranslate.onLocaleOrLanguageChanged
                                value: 9
                            },
                            
                            Option {
                                text: qsTr("Electronic") + Retranslate.onLocaleOrLanguageChanged
                                value: 10
                            },
                            
                            Option {
                                text: qsTr("Hip Hop") + Retranslate.onLocaleOrLanguageChanged
                                value: 11
                            },
                            
                            Option {
                                text: qsTr("Jazz") + Retranslate.onLocaleOrLanguageChanged
                                value: 12
                            },
                            
                            Option {
                                text: qsTr("Lounge") + Retranslate.onLocaleOrLanguageChanged
                                value: 13
                            },
                            
                            Option {
                                text: qsTr("Piano") + Retranslate.onLocaleOrLanguageChanged
                                value: 14
                            },
                            
                            Option {
                                text: qsTr("Rhythm And Blues") + Retranslate.onLocaleOrLanguageChanged
                                value: 15
                            },
                            
                            Option {
                                text: qsTr("Rock") + Retranslate.onLocaleOrLanguageChanged
                                value: 16
                            },
                            
                            Option {
                                text: qsTr("Spoken Word") + Retranslate.onLocaleOrLanguageChanged
                                value: 17
                            }
                        ]
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
    
    function adjustEqualizer() {
        var equalizer = _appConfig.get("equalizer");
        if (equalizer === "") {
            equalizerDropDown.selectedIndex = 0;
        } else {
            equalizerDropDown.selectedIndex = parseInt(equalizer);
        }
    }
    
    onCreationCompleted: {
        adjustNotification();
        adjustScrobbling();
        adjustEqualizer();
    }
}