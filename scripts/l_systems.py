from PIL import Image, ImageDraw
import math

WIDTH = 1024
HEIGHT = 1024


def binary_tree():
    rules = {"1": "11", "0": "1[0]0"}
    seq = "0"
    num_iterations = 6
    for i in range(num_iterations):
        new_seq = ""
        for c in seq:
            if c in rules:
                new_seq += rules[c]
            else:
                new_seq += c
        seq = new_seq

        x = WIDTH / 2
        y = HEIGHT - 1
        angle = -math.pi / 2
        stack = []
        im = Image.new("L", (WIDTH, HEIGHT), 255)
        draw = ImageDraw.Draw(im)
        for c in seq:
            if c == '0' or c == '1':
                d = 10
                next_x = x + d * math.cos(angle)
                next_y = y + d * math.sin(angle)
                draw.line([(x, y), (next_x, next_y)], 0)
                x = next_x
                y = next_y
            elif c == '[':
                stack.append((x, y, angle))
                angle -= math.pi / 4
            elif c == ']':
                x, y, angle = stack.pop()
                angle += math.pi / 4

        if i == num_iterations - 1:
            im.show()


def sierpinski_triangle():
    rules = {"F": "F-G+F+G-F", "G": "GG"}
    seq = "F-G-G"
    num_iterations = 6
    for i in range(num_iterations):
        new_seq = ""
        for c in seq:
            if c in rules:
                new_seq += rules[c]
            else:
                new_seq += c
        seq = new_seq

        x = 0
        y = HEIGHT - 1
        angle = -math.pi / 2
        im = Image.new("L", (WIDTH, HEIGHT), 255)
        draw = ImageDraw.Draw(im)
        for c in seq:
            if c == 'F' or c == 'G':
                d = 15
                next_x = x + d * math.cos(angle)
                next_y = y + d * math.sin(angle)
                draw.line([(x, y), (next_x, next_y)], 0)
                x = next_x
                y = next_y
            elif c == '+':
                angle -= 2 * math.pi / 3
            elif c == '-':
                angle += 2 * math.pi / 3

        if i == num_iterations - 1:
            im.show()


def dragon_curve():
    rules = {"X": "X+YF+", "Y": "-FX-Y"}
    seq = "FX"
    num_iterations = 14
    for i in range(num_iterations):
        new_seq = ""
        for c in seq:
            if c in rules:
                new_seq += rules[c]
            else:
                new_seq += c
        seq = new_seq

        x = WIDTH / 1.35
        y = HEIGHT / 3
        angle = -math.pi / 2
        im = Image.new("L", (WIDTH, HEIGHT), 255)
        draw = ImageDraw.Draw(im)
        for c in seq:
            if c == 'F':
                d = 5
                next_x = x + d * math.cos(angle)
                next_y = y + d * math.sin(angle)
                draw.line([(x, y), (next_x, next_y)], 0)
                x = next_x
                y = next_y
            elif c == '-':
                angle -= math.pi / 2
            elif c == '+':
                angle += math.pi / 2

        if i == num_iterations - 1:
            im.show()


def fractal_plant():
    rules = {"X": "F+[[X]-X]-F[-FX]+X", "F": "FF"}
    seq = "X"
    num_iterations = 6
    for i in range(num_iterations):
        new_seq = ""
        for c in seq:
            if c in rules:
                new_seq += rules[c]
            else:
                new_seq += c
        seq = new_seq

        x = WIDTH / 2
        y = HEIGHT - 1
        angle = -math.pi / 2
        stack = []
        im = Image.new("L", (WIDTH, HEIGHT), 255)
        draw = ImageDraw.Draw(im)
        for c in seq:
            if c == 'F':
                d = 10
                next_x = x + d * math.cos(angle)
                next_y = y + d * math.sin(angle)
                draw.line([(x, y), (next_x, next_y)], 0)
                x = next_x
                y = next_y
            elif c == '[':
                stack.append((x, y, angle))
            elif c == ']':
                x, y, angle = stack.pop()
            elif c == '+':
                angle -= math.pi / 7.2
            elif c == '-':
                angle += math.pi / 7.2

        if i == num_iterations - 1:
            im.show()


def main():
    binary_tree()
    sierpinski_triangle()
    dragon_curve()
    fractal_plant()


if __name__ == "__main__":
    main()
