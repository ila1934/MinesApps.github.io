// let a = 1
// let b = 'Ilya'
// let c = undefined
// console.log(a ?? b ?? c)

// let arr = [1, 2, 3, 4, 5, 'a', 'g', 'r', 'b', '3', 4]
// for(let i = arr.length; i > 0; i--){
//   console.log(arr[i-1])
// }

// let word = ''
// while(word.length < 10){
//   let A = prompt('Введите букву: ')
//   if(typeof(A) == typeof(2)){
//     break
//   }
//   word = word + A
//   console.log(word)
// }

// Используя цикл while, выведите в консоль все четные числа от 2 до 20
// let a = 2
// while (a <= 20) {
//   if(a%2 == 0){
//     console.log(a)
//   }
//   a++
// }

//Используя цикл do...while, найдите и выведите первое число в диапазоне от 20 до 100, которое делится и на 5, и на 7.
let a = 20
do{
  if(a%5 == 0 && a%7 == 0){
    console.log(a)
  }
  a++
}while()