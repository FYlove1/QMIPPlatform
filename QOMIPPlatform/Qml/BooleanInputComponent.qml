import QtQuick 2.15
import QtQuick.Controls 2.15

Column {
    width: parent ? parent.width : 350
    spacing: 5
    
    property string paramName: ""
    property string displayName: paramName
    property bool paramValue: false
    property var dialogRef: null  // 对话框引用
    
    Text {
        width: parent ? parent.width : 350
        text: displayName
        font.bold: true
        font.pixelSize: 13
    }
    
    CheckBox {
        text: "启用"
        checked: paramValue
        
        onCheckedChanged: {
            // 直接更新对话框的params对象
            if (dialogRef && dialogRef.params) {
                dialogRef.params[paramName] = checked
                console.log("[INPUT-BOOLEAN] 参数已修改:", paramName, "=", checked, "当前所有参数:", JSON.stringify(dialogRef.params))
            }
        }
    }
}