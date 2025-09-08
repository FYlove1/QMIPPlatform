import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    width: 600
    height: 450
    color: "#f5f5f5"
    
    // 定义信号，与C++交互
    signal algorithmDeleted(int index)
    signal algorithmParamsChanged(int index, var params)
    signal addAlgorithmRequested()
    
    // 提供刷新方法供C++调用
    function refreshModel() {
        listView.forceLayout()
    }

    // 标题区域
    Rectangle {
        id: header
        width: parent.width
        height: 50
        color: "#3498db"

        Text {
            anchors.centerIn: parent
            text: "算法列表"
            font.pixelSize: 18
            font.bold: true
            color: "white"
        }
    }

    // 添加算法按钮
    Button {
        id: addButton
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.margins: 10
        text: "添加算法"

        onClicked: {
            // 发送信号到C++请求添加算法
            root.addAlgorithmRequested()
        }
    }

    // 算法列表
    ListView {
        id: listView
        anchors.top: addButton.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        clip: true
        spacing: 10
        
        // 使用C++传入的algorithmModel
        model: algorithmModel

        // 自定义委托
        delegate: Rectangle {
            width: listView.width
            height: 80
            radius: 5
            color: "#ffffff"
            border.color: "#dddddd"
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 10
                
                // 算法名称
                ColumnLayout {
                    Layout.preferredWidth: 120
                    spacing: 2
                    
                    Text {
                        text: "算法名称"
                        font.pixelSize: 12
                        color: "#888888"
                    }
                    
                    Text {
                        text: model.name || "未命名算法"
                        font.pixelSize: 14
                        font.bold: true
                    }
                    
                    Text {
                        text: "(ID: " + model.algorithmId + ")"
                        font.pixelSize: 10
                        color: "#999999"
                    }
                }
                
                // 参数区域
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    color: "#f5f5f5"
                    radius: 3
                    
                    Text {
                        anchors.centerIn: parent
                        text: formatParameters(model.params, model.algorithmId)
                        font.pixelSize: 13
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            // 获取所有必需数据，避免对话框中直接访问model
                            // 使用正确的角色值 (Qt::UserRole + 5 对应 ParamsMetaRole)
                            const meta = algorithmModel.data(algorithmModel.index(index, 0), Qt.UserRole + 5) || []
                            const params = model.params || {}
                            // 直接使用model.description，因为roleNames已经映射了
                            const description = model.description || ""
                            const algorithmName = model.name || "未命名算法"
                            
                            console.log("[QML-AlgorithmListView] 准备打开参数编辑对话框:")
                            console.log("[QML-AlgorithmListView] - 索引:", index)
                            console.log("[QML-AlgorithmListView] - 算法名称:", algorithmName)
                            console.log("[QML-AlgorithmListView] - 算法描述:", description)
                            console.log("[QML-AlgorithmListView] - 参数:", JSON.stringify(params))
                            
                            // 传递完整数据给对话框，避免对话框中再次访问model
                            paramEditDialog.openDialog(index, model.algorithmId, params, meta, description, algorithmName)
                        }
                    }
                }
                
                // 删除按钮
                Button {
                    text: "删除"
                    Layout.preferredWidth: 60
                    
                    onClicked: {
                        root.algorithmDeleted(index)
                    }
                }
            }
        }
        
        // 空列表提示
        Text {
            anchors.centerIn: parent
            visible: listView.count === 0
            text: "暂无算法，请点击添加按钮"
            font.pixelSize: 16
            color: "#999999"
        }
    }
    
    // 格式化参数文本 - 只显示参数值，不显示标签
    function formatParameters(params, algorithmId) {
        if (!params) return "无参数";
        
        let result = "";
        let paramValues = [];
        
        // 通用方式：只显示参数值
        for (const key in params) {
            let value = params[key];
            
            // 根据参数类型格式化显示
            if (typeof value === 'number') {
                if (Number.isInteger(value)) {
                    paramValues.push(value.toString());
                } else {
                    paramValues.push(value.toFixed(2));
                }
            } else if (typeof value === 'boolean') {
                paramValues.push(value ? "是" : "否");
            } else {
                paramValues.push(value.toString());
            }
        }
        
        if (paramValues.length === 0) {
            return "无参数";
        }
        
        return paramValues.join(" | ");
    }
    
    // 新的参数编辑对话框
    ParameterEditDialog {
        id: paramEditDialog
        anchors.centerIn: parent


        
        onParametersAccepted: function(algIndex, parameters) {
            // 发送参数更新信号给父组件
            root.algorithmParamsChanged(algIndex, parameters)
        }
    }
}
