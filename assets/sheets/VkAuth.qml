import bb.cascades 1.4

Sheet {
    id: authPage
    
    property string clientId: "6148477"
    property string display: "mobile"
    property string responseType: "token"
    property string apiVersion: "5.68"
    property string scope: "wall"
    
    signal accessTokenAndUserIdReceived(string accessToken, string userId, string apiVersion)
    
    function init() {
        webView.url = "https://oauth.vk.com/authorize?client_id=" + clientId + "&display=" + display + "&response_type=" + responseType 
        + "&v=" + apiVersion + "&scope=" + scope;
    }
    
    Page {
        
        titleBar: TitleBar {
            title: qsTr("VK Login") + Retranslate.onLocaleOrLanguageChanged
            
            dismissAction: ActionItem {
                title: qsTr("Cancel") + Retranslate.onLocaleOrLanguageChanged
                
                onTriggered: {
                    authPage.close();
                }
            }    
        }
        
        ScrollView {
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Fill
                WebView {
                    id: webView     
                    
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment: VerticalAlignment.Fill
                    
                    onUrlChanged: {
                        console.debug(url);
                        var urlStr = url + "";
                        if (urlStr.indexOf("blank.html#access_token") !== -1) {
                            var queryArray = urlStr.split("#")[1].split("&");
                            authPage.accessTokenAndUserIdReceived(queryArray[0].split("=")[1], queryArray[2].split("=")[1], apiVersion);
                            storage.clear();
                        }
                    }
                }
        }
    }
    
    onCreationCompleted: {
        init();
    }
}