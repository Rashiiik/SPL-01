import random

w, h = 10, 10

snake = [(5, 5)]
food = (random.randint(0, w-1), random.randint(0, h-1))
dx, dy = 0, 0

def draw():
    for y in range(h):
        for x in range(w):
            if (x, y) == snake[0]:
                print("O", end=" ")
            elif (x, y) in snake:
                print("o", end=" ")
            elif (x, y) == food:
                print("X", end=" ")
            else:
                print(".", end=" ")
        print()
    print()

while True:
    draw()
    move = input("Move (w/a/s/d): ")

    if move == "w": dx, dy = 0, -1
    elif move == "s": dx, dy = 0, 1
    elif move == "a": dx, dy = -1, 0
    elif move == "d": dx, dy = 1, 0
    else:
        continue

    # Update snake
    new_head = (snake[0][0] + dx, snake[0][1] + dy)

    # Wall collision
    if not (0 <= new_head[0] < w and 0 <= new_head[1] < h):
        print("You hit a wall! Game over.")
        break

    # Self collision
    if new_head in snake:
        print("You ran into yourself! Game over.")
        break

    snake.insert(0, new_head)

    # Food collision
    if new_head == food:
        food = (random.randint(0, w-1), random.randint(0, h-1))
    else:
        snake.pop()
