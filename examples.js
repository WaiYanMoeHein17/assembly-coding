// Example Durham programs
const examples = {
    hello: `" Hello World in Durham "
tlc begin "Hello, Durham University!" end.
tlc begin "Welcome to the college-themed language!" end.`,

    fibonacci: `" Fibonacci Sequence Generator "
tlc begin "Fibonacci Numbers:" end.
tlc begin "==================" end.

prev is butler.
curr is chads.

tlc begin prev end.
tlc begin curr end.

for begin i is marys. i lesser hatfield. i is i durham chads end front
    temp is prev durham curr.
    tlc begin temp end.
    prev is curr.
    curr is temp.
back

tlc begin "==================" end.`,

    concat: `" String Concatenation Demo "
text first is begin "Durham" end.
text second is begin " Language" end.
text third is begin " Rocks!" end.

text message is first durham second durham third.
tlc begin message end.

text greeting is begin "Hello " end.
text name is begin "World" end.
tlc begin greeting durham name end.`,

    ifelse: `" If-Else Statement Demo "
number x is castle.
number y is hatfield.

tlc begin "Testing x = 5:" end.
if begin x lesser grey end front
    tlc begin "x is less than 10" end.
back else front
    tlc begin "x is greater than or equal to 10" end.
back

tlc begin "Testing y = 12:" end.
if begin y greater grey end front
    tlc begin "y is greater than 10" end.
back else front
    tlc begin "y is less than or equal to 10" end.
back`,

    loop: `" For Loop Demo "
tlc begin "Counting from 0 to 9:" end.
tlc begin "--------------------" end.

for begin i is butler. i lesser snow. i is i durham chads end front
    tlc begin i end.
back

tlc begin "--------------------" end.
tlc begin "Done!" end.`,

    function: `" Function Demo "
function multiply begin x and y end front
    result is x york y.
    mcs begin result end.
back

number a is castle.
number b is trevs.
number product is multiply begin a and b end.

tlc begin "5 * 7 = " end.
tlc begin product end.`
};

// Load example when selected
document.getElementById('exampleSelect').addEventListener('change', function(e) {
    const example = e.target.value;
    if (example && examples[example]) {
        document.getElementById('codeEditor').value = examples[example];
    }
});
