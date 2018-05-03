let diff_root _ =
  let x = 1e16 in
  let rand = Random.float(1.0) in
  let x' = (x +. rand) -. rand in
  let y = sqrt(x' +. 1.0) -. sqrt(x') in
  Printf.printf "%e\n" y ;;

diff_root () ;;
diff_root () ;;
