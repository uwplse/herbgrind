let diff_root _ =
  let x = 1e16 in
  let rand = Random.float(1.0) in
  let x' = (x +. rand) -. rand in
  let y = sqrt(x' +. 1.0) -. sqrt(x) in
  if (y > 0.0) then
    Printf.printf "Y is greater than zero!\n"
  else
    Printf.printf "Y is zero or less!\n" ;;

diff_root () ;;
diff_root () ;;
