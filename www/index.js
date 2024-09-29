const xhr = new XMLHttpRequest();

const server_list = [
    document.getElementById("oracle1"),
    document.getElementById("linode1"),
    document.getElementById("linode2"),
]

//サーバーのボタンをクリックした時の動作
function onServerSelect(e){
    for(let i = 0; i < server_list.length; i++){
        server_list[i].style.color = null;
        server_list[i].style.background = null;
    }

    element = e.currentTarget;
    element.style.color = "#fff"
    element.style.background = "#27acd9"

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
                edit_image.dataset.id = i.toString();
                edit_image.width = 32;
                edit_image.height = 32;
                //trタグを作成し、rowを代入
                const tr = document.createElement("tr");
                const port_data = [
                    document.createTextNode(data[i].name),
                    document.createTextNode(data[i].ipaddress),
                    document.createTextNode(data[i].port),
                    document.createTextNode(data[i].protocol),
                    edit_image
                ]

                for (let i0 = 0; i0 < port_data.length; i0++) {
                    const td = document.createElement("td");
                    td.appendChild(port_data[i0]);
                    tr.appendChild(td);
                }

                table_element.appendChild(tr);
            }
        }
    }
}

window.addEventListener("load", () =>{
    //サーバーのボタンを登録
    for(let i = 0; i < server_list.length; i++){
        server_list[i].addEventListener("click", function (event) {
            onServerSelect(event);
        });
    }
});