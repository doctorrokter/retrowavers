import bb.cascades 1.4

Container {
    id: root
    
    property string track: "Privet"
    property string imageUrl: "asset:///images/blur.jpg"
    
    horizontalAlignment: HorizontalAlignment.Fill
    verticalAlignment: VerticalAlignment.Fill
    
    background: ui.palette.background
    
        layout: DockLayout {}
        
        ImageView {
            imageSource: root.imageUrl
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Fill
            scalingMethod: ScalingMethod.AspectFill
            opacity: 0.4
        }
        
        Label {
            text: root.track
            verticalAlignment: VerticalAlignment.Center
            horizontalAlignment: HorizontalAlignment.Center
            textStyle.base: textStyle.style
            textStyle.fontSize: FontSize.Medium
            multiline: true
            
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
