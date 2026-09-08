/*
 * HelpPage - the About page (app Menu -> Help). Structured after BBTelega's
 * HelpSheet (About / Music, every name tapping to its home page in the browser),
 * but keeping the Retrowavers look: the Newtown face from RetroTextStyleDefinition
 * on every label, and ui.palette.primary as the accent.
 *
 * The old page linked the author's name to BlackBerry World, which no longer
 * exists - the name now opens GitHub.
 *
 * One Invocation serves every link: its uri is rewritten before each trigger (see
 * openUrl), which beats declaring a separate Invocation per service.
 */
import bb.cascades 1.4
import bb.platform 1.3
import "../components"
import "../style"

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
            bottomPadding: ui.du(6)

            Header {
                title: qsTr("About") + Retranslate.onLocaleOrLanguageChanged
            }

            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }

                leftPadding: ui.du(2.5)
                rightPadding: ui.du(2.5)
                topMargin: ui.du(2)
                bottomMargin: ui.du(1)

                Label {
                    text: qsTr("Author: ") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: textStyle.style
                }

                Label {
                    text: "Mikhail Chachkouski"
                    textStyle.base: textStyle.style
                    textStyle.color: ui.palette.primary
                    textStyle.fontWeight: FontWeight.W500

                    gestureHandlers: [
                        TapHandler {
                            onTapped: {
                                root.openUrl("https://github.com/doctorrokter");
                            }
                        }
                    ]
                }
            }

            Container {
                leftPadding: ui.du(2.5)
                rightPadding: ui.du(2.5)
                bottomMargin: ui.du(2)

                Label {
                    text: qsTr("App: ") + Retranslate.onLocaleOrLanguageChanged + Application.applicationName
                    textStyle.base: textStyle.style
                }

                Label {
                    topMargin: 0
                    text: qsTr("Version: ") + Retranslate.onLocaleOrLanguageChanged + Application.applicationVersion
                    textStyle.base: textStyle.style
                }

                Label {
                    topMargin: 0
                    text: qsTr("OS: ") + Retranslate.onLocaleOrLanguageChanged + platform.osVersion
                    textStyle.base: textStyle.style
                }
            }

            Header {
                title: qsTr("Music") + Retranslate.onLocaleOrLanguageChanged
            }

            Container {
                topMargin: ui.du(2)
                leftPadding: ui.du(2.5)
                rightPadding: ui.du(2.5)
                bottomMargin: ui.du(2)

                Label {
                    text: "retrowave-radio.ru"
                    textStyle.base: textStyle.style
                    textStyle.color: ui.palette.primary
                    textStyle.fontWeight: FontWeight.W500

                    gestureHandlers: [
                        TapHandler {
                            onTapped: {
                                root.openUrl("https://retrowave-radio.ru");
                            }
                        }
                    ]
                }

                Label {
                    topMargin: 0
                    multiline: true
                    text: qsTr("The Playlist tab plays this station's catalogue, which carries on after the original retrowave.ru went offline.") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: textStyle.style
                }
            }

            Header {
                title: qsTr("Radio") + Retranslate.onLocaleOrLanguageChanged
            }

            Container {
                topMargin: ui.du(2)
                leftPadding: ui.du(2.5)
                rightPadding: ui.du(2.5)

                Label {
                    multiline: true
                    text: qsTr("Stations on the Radio tab come from these services:") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: textStyle.style
                    bottomMargin: ui.du(1)
                }

                Label {
                    topMargin: 0
                    text: "SomaFM"
                    textStyle.base: textStyle.style
                    textStyle.color: ui.palette.primary
                    gestureHandlers: [
                        TapHandler {
                            onTapped: {
                                root.openUrl("https://somafm.com");
                            }
                        }
                    ]
                }

                Label {
                    topMargin: 0
                    text: "Nightride FM"
                    textStyle.base: textStyle.style
                    textStyle.color: ui.palette.primary
                    gestureHandlers: [
                        TapHandler {
                            onTapped: {
                                root.openUrl("https://nightride.fm");
                            }
                        }
                    ]
                }

                Label {
                    topMargin: 0
                    text: "WaveRadio"
                    textStyle.base: textStyle.style
                    textStyle.color: ui.palette.primary
                    gestureHandlers: [
                        TapHandler {
                            onTapped: {
                                root.openUrl("https://waveradio.org");
                            }
                        }
                    ]
                }

                Label {
                    topMargin: 0
                    text: "Nightwave Plaza"
                    textStyle.base: textStyle.style
                    textStyle.color: ui.palette.primary
                    gestureHandlers: [
                        TapHandler {
                            onTapped: {
                                root.openUrl("https://plaza.one");
                            }
                        }
                    ]
                }

                Label {
                    topMargin: 0
                    text: "SynthwaveRadio.eu"
                    textStyle.base: textStyle.style
                    textStyle.color: ui.palette.primary
                    gestureHandlers: [
                        TapHandler {
                            onTapped: {
                                root.openUrl("https://synthwaveradio.eu");
                            }
                        }
                    ]
                }

                Label {
                    topMargin: 0
                    text: "Retrowave.One"
                    textStyle.base: textStyle.style
                    textStyle.color: ui.palette.primary
                    gestureHandlers: [
                        TapHandler {
                            onTapped: {
                                root.openUrl("https://retrowave.one");
                            }
                        }
                    ]
                }
            }
        }
    }

    function openUrl(url) {
        browserInvoke.query.uri = url;
        browserInvoke.query.updateQuery();
        browserInvoke.trigger(browserInvoke.query.invokeActionId);
    }

    attachedObjects: [
        Invocation {
            id: browserInvoke
            query {
                uri: "https://retrowave-radio.ru"
                invokeActionId: "bb.action.OPEN"
                invokeTargetId: "sys.browser"
            }
        },

        PlatformInfo {
            id: platform
        },

        RetroTextStyleDefinition {
            id: textStyle
        }
    ]
}
