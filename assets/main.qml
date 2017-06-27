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
import "components"

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
            id: webView
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
            
            MediaPlayer {
                id: player
            }
        ]
    }
    
    function updateImageUrl() {
        var track = _tracksService.active;
        console.debug(track.bImagePath);
        if (_tracksService.active.bImagePath !== undefined) {
            root.imageUrl = track.bImagePath;
        }
    }
    
    onCreationCompleted: {
        var data = [];
        data.push({type: "player"});
        data.push({type: "list"});
        dataModel.append(data);
        
        _api.load();
        _tracksService.activeChanged.connect(root.updateImageUrl);
    }
}
