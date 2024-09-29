const xhr = new XMLHttpRequest();
let current_server = ""; //現在選択しているサーバーの名前

const server_list = [
    document.getElementById("oracle1"),
    document.getElementById("linode1"),
    document.getElementById("linode2"),
]

//編集ボタンの実装
function editClick(e){
    const table_element = document.getElementById("portList");
    const index = Number(e.target.dataset.index_edit_id)+1;
    const row = table_element.rows[index].getElementsByTagName("td");

    modalOpen();
    is_change = true;
    forms[0].disabled = true;
    send_port_setting_element.style.cursor = "pointer";
    for(let i=0; i < forms.length; i++){
        forms[i].value = row[i].textContent;
    }
}

//ポート情報の削除
function onDeleteButtonClick(e){
    const table_element = document.getElementById("portList");
    const index = Number(e.target.dataset.index_edit_id)+1;
    const row = table_element.rows[index].getElementsByTagName("td");

    xhr.open("DELETE", "/deletePorts");
    xhr.send(JSON.stringify({
        "server": current_server,
        "name": row[0].textContent
    }));
    xhr.onreadystatechange = function (){
        if(xhr.readyState === 4) {
            refresh(document.getElementById(current_server));
            xhr.onreadystatechange = null;
        }
    }
}


function refresh(button_element){

    for(let i = 0; i < server_list.length; i++){
        server_list[i].style.color = null;
        server_list[i].style.background = null;
    }

    const element = button_element;
    element.style.color = "#fff"
    element.style.background = "#27acd9"
    current_server = element.id;

    //サーバーからデータを取得
    xhr.open("POST", "/ports");
    xhr.setRequestHeader('content-type', 'application/json;charset=UTF-8');
    xhr.send(JSON.stringify({
        "serverID": element.id
    }));
    xhr.onreadystatechange = function (){
        if(xhr.readyState === 4) {
            const data = JSON.parse(xhr.responseText); //受信したポートデータを代入
            const table_element = document.getElementById("portList");
            //テーブルのデータを削除
            while (table_element.rows.length !== 1){
                table_element.deleteRow(table_element.rows.length-1);
            }
            //テーブルを作成
            for (let i = 0; i < data.length; i++) {
                //編集イメージを作成
                const edit_image = document.createElement("img");
                edit_image.src = "contents/edit.png";
                edit_image.id = "edit-btn";
                edit_image.dataset.index_edit_id = i.toString();
                edit_image.width = 32;
                edit_image.height = 32;
                const delete_image = document.createElement("img");
                delete_image.src = "contents/delete.png";
                delete_image.id = "delete-btn";
                delete_image.dataset.index_edit_id = i.toString();
                delete_image.width = 32;
                delete_image.height = 32;
                //trタグを作成し、rowを代入
                const tr = document.createElement("tr");
                const port_data = [
                    document.createTextNode(data[i].name),
                    document.createTextNode(data[i].ipaddress),
                    document.createTextNode(data[i].port),
                    document.createTextNode(data[i].protocol),
                    edit_image,
                    delete_image
                ]

                for (let i0 = 0; i0 < port_data.length; i0++) {
                    const td = document.createElement("td");
                    td.appendChild(port_data[i0]);
                    tr.appendChild(td);
                }
                //編集ボタンのクリックイベントをリッスン
                edit_image.addEventListener("click", function (event){
                    editClick(event);
                })

                //削除ボタンのイベントをリッスン
                delete_image.addEventListener("click", function (event){
                    onDeleteButtonClick(event);
                })
                table_element.appendChild(tr);
            }
            xhr.onreadystatechange = null;
        }
    }
}

//サーバーのボタンをクリックした時の動作
function onServerSelect(e){
    refresh(e.currentTarget);
}

window.addEventListener("load", () =>{
    //サーバーのボタンを登録
    for(let i = 0; i < server_list.length; i++){
        server_list[i].addEventListener("click", function (event) {
            onServerSelect(event);
        });
    }
});