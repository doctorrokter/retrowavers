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
import "components"
import "pages"

NavigationPane {
    id: navigation
    
    Menu.definition: MenuDefinition {
        helpAction: HelpActionItem {
            onTriggered: {
                var hp = helpPage.createObject();
                navigation.push(hp);
            }            
        }
        
        actions: [
            ActionItem {
                title: qsTr("LastFM account") + Retranslate.onLocaleOrLanguageChanged
                imageSource: "asset:///images/ic_sign_out.png"
                
                onTriggered: {
                    var fm = lastFm.createObject();
                    navigation.push(fm);
                }
            },
            
            ActionItem {
                title: qsTr("Send feedback") + Retranslate.onLocaleOrLanguageChanged
                imageSource: "asset:///images/ic_feedback.png"
                
                onTriggered: {
                    invokeFeedback.trigger(invokeFeedback.query.invokeActionId);
                }
            }
        ]
    }
    
    Page {
        id: root
    
        property string imageUrl: "asset:///images/blur.jpg"
    
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
            
                horizontalAlignment: HorizontalAlignment.Fill
                verticalAlignment: VerticalAlignment.Fill
            
                dataModel: ArrayDataModel {
                    id: dataModel
                }
            
                layout: StackListLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
            
                flickMode: FlickMode.SingleItem
            
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
                            Player {}
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
    
        onCreationCompleted: {
            var data = [];
            data.push({type: "player"});
            data.push({type: "list"});
            dataModel.append(data);
        
            _api.load();
            _tracksService.activeChanged.connect(root.updateImageUrl);
        
            Application.thumbnail.connect(function() {
                Application.setCover(cover);    
            });
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
        }
    ]
}
