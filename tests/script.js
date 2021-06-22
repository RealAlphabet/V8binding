
// Torus.registerEvents(PLAYER_JOIN, function (event) {
//     event.cancel        = true;
//     event.cancel_reason = "Tu n'es pas autorizay !";
// });

let i = 0;

function a() {
    let obj = {
        a: 1 % 2,
        function () {

        },
        b: i * 2
    };

    return obj;
}

scheduleTicks(function () {
    i++;
    println(`${i}`);
    let obj = a();

    return obj.b;
});

