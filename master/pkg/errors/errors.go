package errors

import "os"

func ExitIfErr(err error) {
  if err != nil {
    println("Error: ", err.Error())
    os.Exit(1)
  }
}
