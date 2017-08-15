import bb.cascades 1.4

Sheet {
    id: authPage
    
    property string clientId: "1878917592428734"
    property string responseType: "token"
    property string apiVersion: "2.10"
    property string scope: "publish_actions"
    property string redirectUri: "https://www.facebook.com/connect/login_success.html"
    
    signal accessTokenAndUserIdReceived(string accessToken, string apiVersion)
    
    function init() {
        webView.url = "https://www.facebook.com/v" + apiVersion + "/dialog/oauth?client_id=" + clientId + "&response_type=" + responseType 
        + "&scope=" + scope + "&redirect_uri=" + redirectUri;
    }
    
    Page {
        
        titleBar: TitleBar {
            title: qsTr("Facebook Login") + Retranslate.onLocaleOrLanguageChanged
            
            dismissAction: ActionItem {
                title: qsTr("Cancel") + Retranslate.onLocaleOrLanguageChanged
                
                onTriggered: {
                    authPage.close();
                }
            }    
        }
        
        Container {
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Fill
            WebView {
                id: webView     
                
                preferredWidth: webviewLUH.layoutFrame.width
                preferredHeight: webviewLUH.layoutFrame.height
                
                onUrlChanged: {
                    console.debug(url);
                    var urlStr = url + "";
                    if (urlStr.substring(0, redirectUri.length) === redirectUri) {
                        var queryArray = urlStr.split("#")[1].split("&");
                        authPage.accessTokenAndUserIdReceived(queryArray[0].split("=")[1], apiVersion);
                    }
                }
            }
            
            attachedObjects: [
                LayoutUpdateHandler {
                    id: webviewLUH
                }
            ]
        }
    }
    
    onCreationCompleted: {
        init();
    }
}