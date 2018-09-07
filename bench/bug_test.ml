
let x = 1e16 in
    let rand = Random.float(1.0) in
    let y = (x +. rand) -. rand in
    Printf.printf "%e\n" (sqrt(y)) ;;
