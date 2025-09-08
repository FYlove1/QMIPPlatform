import QtQuick 2.15
import QtQuick.Controls 2.15

Column {
    width: parent.width
    spacing: 5
    
    property string paramName: ""
    property string displayName: paramName
    property var paramValue: ""
    property var paramsObject: ({})
    
    Text {
        width: parent.width
        text: displayName
        font.bold: true
        font.pixelSize: 13
    }
    
    TextField {
        width: parent.width
        text: paramValue !== undefined ? paramValue.toString() : ""
        placeholderText: "输入文本"
        
        onTextChanged: {
            paramsObject[paramName] = text
        }
    }
}