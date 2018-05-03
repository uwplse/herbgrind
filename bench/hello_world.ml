let say_hello _ =
  let rand = Random.float(1.0) in
  let x = sqrt(rand) in
  Printf.printf "Hello world %e!\n" x;;

say_hello () ;;
