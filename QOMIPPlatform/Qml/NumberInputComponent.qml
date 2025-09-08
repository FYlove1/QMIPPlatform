import QtQuick 2.15
import QtQuick.Controls 2.15

Column {
    width: parent ? parent.width : 350
    spacing: 5
    
    property string paramName: ""
    property string displayName: paramName
    property var paramValue
    property bool isDouble: false
    property var dialogRef: null  // 对话框引用
    
    Text {
        width: parent ? parent.width : 350
        text: displayName
        font.bold: true
        font.pixelSize: 13
    }
    
    TextField {
        id: numberField
        width: parent ? parent.width : 350
        text: paramValue !== undefined ? paramValue.toString() : ""
        placeholderText: isDouble ? "输入小数" : "输入整数"
        
        // 验证器
        validator: isDouble ? doubleValidator : intValidator
        
        // 小数验证器
        DoubleValidator {
            id: doubleValidator
            notation: DoubleValidator.StandardNotation
        }
        
        // 整数验证器
        IntValidator {
            id: intValidator
        }
        
        // 高亮错误
        color: acceptableInput || text === "" ? "#000000" : "#ff0000"
        
        // 更新参数
        onTextChanged: {
            if (acceptableInput || text === "") {
                var newValue = text === "" ? (isDouble ? 0.0 : 0) : (isDouble ? parseFloat(text) : parseInt(text))
                // 直接更新对话框的params对象
                if (dialogRef && dialogRef.params) {
                    dialogRef.params[paramName] = newValue
                    console.log("[INPUT-NUMBER] 参数已修改:", paramName, "=", newValue, "当前所有参数:", JSON.stringify(dialogRef.params))
                }
            }
        }
    }
}