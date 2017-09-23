/*
 * Copyright (c) 2011-2015 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import bb.cascades 1.4
import bb.multimedia 1.4
import bb.system 1.2
import chachkouski.util 1.0
import "components"
import "pages"

NavigationPane {
    id: navigation
    
    Menu.definition: MenuDefinition {
        helpAction: HelpActionItem {
            onTriggered: {
                var hp = helpPage.createObject();
                navigation.push(hp);
                Application.menuEnabled = false;
            }            
        }
        
        settingsAction: SettingsActionItem {
            onTriggered: {
                var sp = settingsPage.createObject();
                navigation.push(sp);
                Application.menuEnabled = false;
            }
        }
        
        actions: [
            ActionItem {
                id: rateAppAction
                
                title: qsTr("Rate app") + Retranslate.onLocaleOrLanguageChanged
                imageSource: "asset:///images/ic_blackberry.png"
                
                onTriggered: {
                    _appConfig.set("app_rated", "true");
                    bbwInvoke.trigger(bbwInvoke.query.invokeActionId);
                }
            },
            
            ActionItem {
                title: qsTr("Send feedback") + Retranslate.onLocaleOrLanguageChanged
                imageSource: "asset:///images/ic_feedback.png"
                
                onTriggered: {
                    invokeFeedback.trigger(invokeFeedback.query.invokeActionId);
                }
            },
            
            ActionItem {
                title: qsTr("LastFM account") + Retranslate.onLocaleOrLanguageChanged
                imageSource: "asset:///images/ic_sign_out.png"
                
                onTriggered: {
                    var fm = lastFm.createObject();
                    navigation.push(fm);
                }
            }
        ]
    }
    
    Page {
        id: root
    
        property string imageUrl: "asset:///images/blur.jpg"
        property bool controlsShown: false
    
        Container {
        
            id: rootContainer
        
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Fill
        
            background: ui.palette.plain
        
            layout: DockLayout {}
        
            ImageView {
                imageSource: "asset:///images/palms-bg.png"
                horizontalAlignment: HorizontalAlignment.Fill
                verticalAlignment: VerticalAlignment.Fill
                scalingMethod: ScalingMethod.AspectFill
            }
        
            ImageView {
                horizontalAlignment: HorizontalAlignment.Fill
                verticalAlignment: VerticalAlignment.Fill
            
                scalingMethod: ScalingMethod.AspectFill
                imageSource: root.imageUrl
            
                opacity: 0.4
            }
        
            ListView {           
                id: rootList
            
                property double width: 0
                property double height: 0
                property bool playing: true
                property bool controlsShown: false
            
                horizontalAlignment: HorizontalAlignment.Fill
                verticalAlignment: VerticalAlignment.Fill
            
                dataModel: ArrayDataModel {
                    id: dataModel
                }
            
                layout: StackListLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
            
                flickMode: FlickMode.SingleItem
                
                onTriggered: {
                    rootList.controlsShown = !rootList.controlsShown;
                    var data = dataModel.data(indexPath);
                    console.debug("controls shown: ", rootList.controlsShown, " type: ", data.type);
                    if (data.type === "player") {
                        root.controlsShown = !root.controlsShown;
                    }
                }
            
                function itemType(data, indexPath) {
                    return data.type;
                }
            
                listItemComponents: [
                    ListItemComponent {
                        type: "player"
                        CustomListItem {
                            dividerVisible: false
                            highlightAppearance: HighlightAppearance.None
                        
                            preferredWidth: ListItem.view.width
                            preferredHeight: ListItem.view.height
                            
                            Player {
                                controlsShown: ListItem.view.controlsShown
                            }
                        }
                    },
                
                    ListItemComponent {
                        type: "list"
                        CustomListItem {
                            dividerVisible: false
                            highlightAppearance: HighlightAppearance.None
                        
                            preferredWidth: ListItem.view.width
                            preferredHeight: ListItem.view.height
                            List {}
                        }
                    }
                ]
            }
            
//            Controls {
//                id: controlsContainer
//                shown: root.controlsShown
//                verticalAlignment: VerticalAlignment.Bottom
//            }
        
            attachedObjects: [
                LayoutUpdateHandler {
                    id: rootLUH
                
                    onLayoutFrameChanged: {
                        rootList.width = layoutFrame.width;
                        rootList.height = layoutFrame.height;
                    }
                },
            
                SceneCover {
                    id: cover
                
                    property string track: "Retrowavers"
                    property string imageUrl: "asset:///images/blur.jpg"
                
                    content: Cover {
                        track: cover.track
                        imageUrl: cover.imageUrl
                    }
                }
            ]
        }
    
        function updateImageUrl() {
            var track = _tracksService.active;
            var bImg = _tracksService.active.bImagePath;
            var blurBg = bImg !== undefined && bImg !== "" ? bImg : root.imageUrl;

            cover.track = track.title;
            cover.imageUrl = blurBg;
            if (Application.isThumbnailed()) {
                Application.setCover(cover);
            } else {
                root.imageUrl = blurBg;
            }
        }
        
        function changeBlurImage(id, imagePath) {
            var track = _tracksService.active;
            if (track && track.id === id) {
                cover.imageUrl = imagePath;
                root.imageUrl = imagePath;
            }
        }
    
        onCreationCompleted: {
            var data = [];
            data.push({type: "player"});
            data.push({type: "list"});
            dataModel.append(data);
        
            _tracksService.activeChanged.connect(root.updateImageUrl);
            _tracksService.blurImageChanged.connect(root.changeBlurImage);
        
            Application.thumbnail.connect(function() {
                Application.setCover(cover);    
            });
            
            timer.start();
            
            var startCount = _appConfig.get("start_count");
            if (startCount === "") {
                startCount = 1;
            } else {
                startCount = parseInt(startCount);
                startCount++;
            }
            
            var appRated = _appConfig.get("app_rated");
            if ((startCount === 2 || startCount % 5 === 0) && (appRated === "" || appRated === "false")) {
                dialog.show();
            }
            
            _appConfig.set("start_count", startCount);
        }
    }
    
    onPopTransitionEnded: {
        if (page.cleanUp !== undefined) {
            page.cleanUp();
        }
        page.destroy();
        Application.menuEnabled = true;
    }
    
    attachedObjects: [
        ComponentDefinition {
            id: helpPage
            HelpPage {}
        },
        
        ComponentDefinition {
            id: settingsPage
            SettingsPage {}    
        },
        
        ComponentDefinition {
            id: lastFm
            LastFMAuth {}    
        },
        
        Invocation {
            id: invokeFeedback
            query {
                uri: "mailto:retrowavers.bbapp@gmail.com?subject=Retrowavers:%20Feedback"
                invokeActionId: "bb.action.SENDEMAIL"
                invokeTargetId: "sys.pim.uib.email.hybridcomposer"
            }
        },
        
        Invocation {
            id: bbwInvoke
            query {
                uri: "appworld://content/60003994"
                invokeActionId: "bb.action.OPEN"
                invokeTargetId: "sys.appworld"
            }
        },
        
        Timer {
            id: timer
            
            interval: 1000
            singleShot: true
            
            onTimeout: {
                if (_app.online) {
                    _api.load();
                } else {
                    _app.toast(qsTr("No internet connection") + Retranslate.onLocaleOrLanguageChanged);
                }
            }
        },
        
        SystemDialog {
            id: dialog
            
            title: qsTr("Love this app?") + Retranslate.onLocaleOrLanguageChanged
            body: dialog.body = qsTr("This app is free and will be free without any annoying ads and payments. " + 
                "But only one thing I would ask you is to leave a comment in BlackBerry World. " +
                "It will help other people discover this app and increase my motivation to write other applications. " +
                "Thanks for choosing this app!") + Retranslate.onLocaleOrLanguageChanged
            
            confirmButton {
                label: qsTr("Rate app!") + Retranslate.onLocaleOrLanguageChanged
            }
            
            cancelButton {
                label: qsTr("Not now") + Retranslate.onLocaleOrLanguageChanged
            }
            
            onFinished: {
                if (value === 2) {
                    rateAppAction.triggered();
                }
            }
        }
    ]
}
