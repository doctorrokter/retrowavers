import bb.cascades 1.4

Container {
    id: root 
    
    horizontalAlignment: HorizontalAlignment.Fill
    
    layout: DockLayout {}
    
    ListView {
        id: songsList
        
        dataModel: ArrayDataModel {
            id: songsDataModel
        }
        
        onTriggered: {
            clearSelection();
            select(indexPath);
            var data = songsDataModel.data(indexPath);
            _tracksController.play(data);
        }
        
        function loaded() {
            spinner.stop();
            _api.loaded.disconnect(songsList.loaded);
        }
        
        attachedObjects: [
            ListScrollStateHandler {
                onScrollingChanged: {
                    if (atEnd) {
                        if (!spinner.running) {
                            spinner.start();
                            _api.loaded.connect(songsList.loaded);
                            _api.load();
                        }
                    }
                }
            }
        ]
        
        listItemComponents: [
            ListItemComponent {
                CustomListItem {
                    Container {
                        layout: StackLayout {
                            orientation: LayoutOrientation.LeftToRight
                        }
                        
                        verticalAlignment: VerticalAlignment.Center
                        
                        leftPadding: ui.du(2)
                        rightPadding: ui.du(2)
                        
                        Label {
                            text: ListItemData.title
                            textStyle.base: textStyle.style
                            verticalAlignment: VerticalAlignment.Center
                            
                            layoutProperties: StackLayoutProperties {
                                spaceQuota: 1
                            }
                        }
                        
                        Label {
                            text: getMediaTime(ListItemData.duration)
                            textStyle.base: textStyle.style
                            textStyle.fontSize: FontSize.XXSmall
                            verticalAlignment: VerticalAlignment.Center
                        }
                    }
                    
                    function getMediaTime(time) {
                        var seconds = Math.round(time / 1000);
                        var h = Math.floor(seconds / 3600) < 10 ? '0' + Math.floor(seconds / 3600) : Math.floor(seconds / 3600);
                        var m = Math.floor((seconds / 60) - (h * 60)) < 10 ? '0' + Math.floor((seconds / 60) - (h * 60)) : Math.floor((seconds / 60) - (h * 60));
                        var s = Math.floor(seconds - (m * 60) - (h * 3600)) < 10 ? '0' + Math.floor(seconds - (m * 60) - (h * 3600)) : Math.floor(seconds - (m * 60) - (h * 3600));
                        return m + ':' + s;
                    }
                    
                    attachedObjects: [
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
            }
        ]
    }
    
    ActivityIndicator {
        id: spinner
        
        verticalAlignment: VerticalAlignment.Center
        horizontalAlignment: HorizontalAlignment.Center
        
        minWidth: ui.du(20)
    }
    
    function addTracks(tracks) {
        songsDataModel.append(tracks);
    }
    
    function onPlayed() {
        songsList.clearSelection();
        for (var i = 0; i < songsDataModel.size(); i++) {
            var track = songsDataModel.value(i);
            if (track.id === _tracksService.active.id) {
                songsList.select([i]);
            }
        }
    }
    
    function updateImagePath(id, imagePath) {
        for (var i = 0; i < songsDataModel.size(); i++) {
            var data = songsDataModel.value(i);   
            if (data.id === id) {
                data.imagePath = "file://" + imagePath;
                songsDataModel.replace(i, data);
            }    
        }
    }
    
    onCreationCompleted: {
//        var data = [];
//        data.push({title: "OGRE – Flex In", duration: 60000});
//        data.push({title: "Waveshaper - Dominator", duration: 125000});
//        data.push({title: "Oscillian - Ad Astra", duration: 350000});
//        songsDataModel.append(data);
        songsDataModel.clear();
        _api.loaded.connect(addTracks);
        _tracksService.activeChanged.connect(root.onPlayed);
        _tracksService.imageChanged.connect(root.updateImagePath);
    }
}
