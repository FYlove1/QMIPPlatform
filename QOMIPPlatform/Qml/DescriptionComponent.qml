import QtQuick 2.15

Column {
    width: parent ? parent.width : 350
    spacing: 5
    property string description: ""
    
    Text {
        width: parent ? parent.width : 350
        text: "算法描述"
        font.bold: true
        font.pixelSize: 14
    }
    
    Rectangle {
        width: parent ? parent.width : 350
        height: descText.contentHeight + 20
        color: "#f5f5f5"
        radius: 4
        
        Text {
            id: descText
            anchors.fill: parent
            anchors.margins: 10
            text: description
            wrapMode: Text.WordWrap
            font.pixelSize: 12
        }
    }
    
    Rectangle {
        width: parent ? parent.width : 350
        height: 1
        color: "#dddddd"
        anchors.topMargin: 10
    }
}