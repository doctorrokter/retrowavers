import bb.cascades 1.4
import "../style"
import "../components"

Container {
    id: root 
    
    property int touchY: 0
    
    horizontalAlignment: HorizontalAlignment.Fill
    layout: DockLayout {}
    
    ListView {
        id: songsList
        
        scrollRole: ScrollRole.Main
        topPadding: subHeader.height
        
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
        
        onTouch: {
            if (event.isDown()) {
                root.touchY = event.windowY;
            }
            
            if (event.isMove()) {
                if (root.touchY > (event.windowY + 5)) { // scroll up
                    subHeader.isShown = false;
                } else if (root.touchY < (event.windowY - 5)) { // scroll down
                    subHeader.isShown = true;
                }
            }
        }
        
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
                        
                        ImageView {
                            visible: ListItemData.favourite
                            imageSource: "asset:///images/heart_filled.png"
                            maxWidth: ui.du(4)
                            maxHeight: ui.du(4)
                            verticalAlignment: VerticalAlignment.Center
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
                        RetroTextStyleDefinition {
                            id: textStyle
                        }
                    ]
                }
            }
        ]
    }
    
    Subheader {
        id: subHeader
        
        option1: qsTr("Playlist") + Retranslate.onLocaleOrLanguageChanged
        option2: qsTr("Favourite") + Retranslate.onLocaleOrLanguageChanged
        
        onOption1Selected: {
            //            filmsContainer.visible = true;
            //            cinemasContainer.visible = false;
        }
        
        onOption2Selected: {
            //            filmsContainer.visible = false;
            //            cinemasContainer.visible = true;
        }
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
//        data.push({title: "OGRE – Flex In", duration: 60000, favourite: false});
//        data.push({title: "Waveshaper - Dominator sdfsdfs sdfsdf", duration: 125000, favourite: false});
//        data.push({title: "Oscillian - Ad Astra", duration: 350000, favourite: false});
//        data.push({title: "Oscillian - Tarakan", duration: 350000, favourite: true});
//        songsDataModel.append(data);
        songsDataModel.clear();
        _api.loaded.connect(addTracks);
        _tracksService.activeChanged.connect(root.onPlayed);
        _tracksService.imageChanged.connect(root.updateImagePath);
    }
}
