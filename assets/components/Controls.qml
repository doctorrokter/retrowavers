import bb.cascades 1.4
import "../sheets"

Container {
        id: root
    
        property string backgroundImg: "asset:///images/blur.jpg"
        property double imageSize: 13
        property bool shown: false
    
        verticalAlignment: VerticalAlignment.Bottom
        horizontalAlignment: HorizontalAlignment.Fill
        maxHeight: ui.du(35)
        translationY: 0
        background: ui.palette.background
    
        layout: DockLayout {}
    
        attachedObjects: [
            LayoutUpdateHandler {
                id: controlsLUH
            
                onLayoutFrameChanged: {
                    root.translationY = layoutFrame.height;
                }
            },
            
            ComponentDefinition {
                id: vkAuth
                VkAuth {}
            },
            
            ComponentDefinition {
                id: fbAuth
                FBAuth {}
            }
        ]
    
        onShownChanged: {
            if (shown) {
                root.translationY = 0;
            } else {
                root.translationY = controlsLUH.layoutFrame.height;
            }
        }
    
        ImageView {
            horizontalAlignment: HorizontalAlignment.Fill
            imageSource: root.backgroundImg
            scalingMethod: ScalingMethod.AspectFill
            opacity: 0.75
        }

        Container {
    
        horizontalAlignment: HorizontalAlignment.Fill
    
        margin.leftOffset: ui.du(2)
        margin.topOffset: ui.du(2)
        margin.rightOffset: ui.du(2)
    
        Slider {
            id: slider
        }
    
        Container {
            id: socialContainer
        
            horizontalAlignment: HorizontalAlignment.Center
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
        
            ImageView {
                id: vkImage
                
                imageSource: "asset:///images/ic_vk.png"
                maxWidth: ui.du(root.imageSize);
                maxHeight: ui.du(root.imageSize);
                
                function shareWithVk() {
                    console.debug("===>>> POST TO VK");
                    if (_tracksService.active !== undefined) {
                        var track = _tracksService.active.toMap();
                        _vkController.share(track);
                    }
                }
                
                gestureHandlers: [
                    TapHandler {
                        onTapped: {
                            if (_appConfig.get("vk_access_token") === "") {
                                var vkSheet = vkAuth.createObject();
                                vkSheet.accessTokenAndUserIdReceived.connect(function(accessToken, userId, apiVersion) {
                                    vkSheet.close();
                                    _appConfig.set("vk_access_token", accessToken);
                                    _appConfig.set("vk_user_id", userId);
                                    _appConfig.set("vk_api_version", apiVersion);
                                    vkImage.shareWithVk();
                                });
                                vkSheet.open();
                            } else {
                                vkImage.shareWithVk();
                            }
                        }
                    }
                ]
            }
        
            ImageView {
                id: fbImage
                
                margin.leftOffset: ui.du(7)
                imageSource: "asset:///images/ic_facebook.png"
                maxWidth: ui.du(root.imageSize);
                maxHeight: ui.du(root.imageSize);
                
                function shareWithFB() {
                    console.debug("===>>> POST TO FB");
                    if (_tracksService.active !== undefined) {
                        var track = _tracksService.active.toMap();
                        _fbController.share(track);
                    }
                }
                
                gestureHandlers: [
                    TapHandler {
                        onTapped: {
                            if (_appConfig.get("fb_access_token") === "") {
                                var fbSheet = fbAuth.createObject();
                                fbSheet.accessTokenAndUserIdReceived.connect(function(accessToken, apiVersion) {
                                        fbSheet.close();
                                        _appConfig.set("fb_access_token", accessToken);
                                        _appConfig.set("fb_api_version", apiVersion);
                                        fbImage.shareWithFB();
                                });
                                fbSheet.open();
                            } else {
                                fbImage.shareWithFB();
                            }
                        }
                    }
                ]
            }
        
            ImageView {
                margin.leftOffset: ui.du(7)
                imageSource: "asset:///images/ic_twitter.png"
                maxWidth: ui.du(root.imageSize);
                maxHeight: ui.du(root.imageSize);
            }
        }
    }
        
        onCreationCompleted: {
            _tracksService.activeChanged.connect(root.updateImageUrl);
        }
        
        function updateImageUrl() {
            var bImg = _tracksService.active.bImagePath;
            if (bImg !== undefined && bImg !== "") {
                root.backgroundImg = bImg;
            }
        }
}