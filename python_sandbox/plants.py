import turtle as t
import time

rules = {
    # "F" : "F-F+F+FF-F-F+F",
    # "F" : "FF-F-F-F-FF",
    # "X" : "F[+F]F[-F]F",
    # "X" : "FF-[-F+F+F]+[+F-F-F]",
    # "F" : "[++F][+F][F][-F][--F]",
    # "F" : "[++X][+F][F][-F][--X]",
    # "X": "FFF",
    # "X": "F",

    # "F" : "F+F-F-F+F",
    "X" : "F[+FX][-FX][++X][--X]FFX",
    # "P" : "F[+FX][-FX]F",

    "F" : "FF",
    "l" : "l+r+",
    "r" : "-l-r",
    "[" : "[",
    "]" : "]",
    "f" : "f",
    "+" : "+",
    "-" : "-"
}

stack = []

def F(a, d):
    t.pd()
    t.fd(d)

def f(a, d):
    t.pu()
    t.fd(d)

def p(a, d):
    t.rt(a)

def m(a, d):
    t.lt(a)

def push(a, d):
    stack.append((t.xcor(), t.ycor(), t.heading()))


def pop(a, d):
    state = stack.pop()
    t.pu()
    t.goto(state[0], state[1])
    t.seth(state[2])
    t.pd()


mappings = {
    "F" : F,
    "l" : F,
    "r" : F,
    "f" : f,
    "+" : p,
    "-" : m,
    "[": push,
    "]" : pop
}

def draw(wn, in_str, a, d):
    # Clear canvas
    t.clear()
    t.pu()
    t.goto(0, -200)
    t.seth(90)
    for c in in_str:
        mappings.get(c, F)(a, d)
    wn.update()


def dol(in_str):
    ret_str = ""
    for c in in_str:
        ret_str += rules.get(c, c)
    return ret_str

def main(wn):
    # Initialize
    # t.speed("fastest")
    # t.delay(0)

    t.ht()
    plant = "X"
    angle = 34.5
    length = 50

    print(plant)
    draw(wn, plant, angle, length)

    time.sleep(1)


    for i in range(6):

        # Compute plant
        plant = dol(plant)

        # Draw it
        # print(plant)
        draw(wn, plant, angle, length)
        wn.update()
        time.sleep(5)

        # Update
        length /= 1.7




if __name__ == "__main__":
    wn = t.Screen()
    wn.tracer(0)

    main(wn)
    wn.mainloop()