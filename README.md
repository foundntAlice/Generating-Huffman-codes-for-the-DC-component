#This is a part of the Opensource Software course's assignment!

**Some significant changes compared to the skeleton code:**
- Added 1 more turtle 
- Added a feature to help the red turtle automatically chase (not very optimal)
- Adjusted the code to force to turtles not running out of screen margins
- Added score concept and a timer

**The following is explaination for the Turtle Runaway game!**

*Plot:
The main character is an agressive red turtle. Like many of us's mom, his mom compares him to a very briliant blue turtle all the time. Very upset, he decided to 
chase and bully the poor blue turtle after school. The thing is... the blue turtle is always side by side with his mom so our main character must find a way to 
get closer to the blue turtle but also not get caught by the blue turtle's mom.

*Implementation:
Below is the function to get the distance of two 2D points (squared)
```
    def distance(x1, y1, x2, y2): 
        res = (x1 - x2)**2 + (y1 - y2)**2
        return (int)(res)
```
The __init__ function:
```
    def __init__(self, canvas, runner, chaser, runner_mom, catch_radius=20, init_dist=600, mom_range=50):
        self.canvas = canvas
        self.runner = runner
        self.chaser = chaser
        self.runner_mom = runner_mom
        self.catch_radius2 = catch_radius**2
        self.mom_range2 = mom_range**2

        # Initialize 'runner' and 'chaser'
        self.runner.shape('turtle')
        self.runner.color('blue')
        self.runner.penup()
        self.runner.setx(-init_dist / 2)

        self.chaser.shape('turtle')
        self.chaser.color('red')
        self.chaser.penup()
        self.chaser.setx(+init_dist / 2)
        self.chaser.setheading(180)

        self.runner_mom.shape('turtle')
        self.runner_mom.color('yellow')
        self.runner_mom.penup()
        self.runner_mom.setx(0)

        # Instantiate an another turtle for drawing
        self.drawer = turtle.RawTurtle(canvas)
        self.drawer.hideturtle()
        self.drawer.penup() 
```
There are 3 main objects, **runner**(the blue turtle), **chaser**(the red turtle) and **runner_mom**(the blue turtle's mom).
The function **it_caught()** returns **True** when the chaser caught up the runner!
```
    def is_caught(self):
        p = self.runner.pos()
        q = self.chaser.pos()
        dx, dy = p[0] - q[0], p[1] - q[1]
        return dx**2 + dy**2 < self.catch_radius2
```
The function **mom_found()** returns **True** when the runner's mom caught up the chaser! Note that this range is much larger than the range the chaser can catch the runner!
```
    def mom_found(self):
        p = self.runner_mom.pos()
        q = self.chaser.pos()
        dx, dy = p[0] - q[0], p[1] - q[1]
        return dx**2 + dy**2 < self.mom_range2 
```
The function **get_score()** returns the score of the chaser. The potential maximum score is 100 initially but decreases gradually as the time passes!
```
    def get_score(self):
        max_score = 100
        penalty = (int)(time.time() - self.start_time)
        return max_score - penalty
```
This is how the game works!
```
    def start(self, ai_timer_msec=50):
        self.ai_timer_msec = ai_timer_msec
        self.start_time = time.time()
        self.canvas.ontimer(self.step, self.ai_timer_msec)

    def step(self):
        self.runner.run_ai(self.chaser)
        #the runner's mom always tries to get closer to her son
        self.runner_mom.mom_move(self.runner)
        self.chaser.auto_chase(self.runner, self.runner_mom)
        #self.chaser.run_ai()
        

        # TODO: You can do something here.
        self.drawer.undo()
        self.drawer.penup()
        self.drawer.setpos(-300, 300)
        elapse = time.time() - self.start_time
        
        if self.mom_found():
            self.drawer.write("The runner's mom got you, 0 points!")
        #time up
        elif elapse > 100:
            self.drawer.write(f"Time is up, you get 0 point!")
        #when the chaser caught the runner!
        elif self.is_caught():
            self.drawer.write(f"The poor blue turtle is catched, you get {self.get_score()} points!")
        #when nothing special happens
        else:
            self.drawer.write(f'Is catched? {self.is_caught()} / Elapse: {elapse:.0f} / Current Score: {self.get_score()}')
            self.canvas.ontimer(self.step, self.ai_timer_msec)
```
**self.runner.run_ai(self.chaser)** helps the runner to move randomly 
**self.runner_mom.mom_move(self.runner)** helps the runner's mom to be side by side with her son
**self.chaser.auto_chase(self.runner, self.runner_mom)** let the chaser run automatically (not very optimal but the result is much better than manual moves)
**elapse = time.time() - self.start_time** calculates the elapse time from when the game started
The rest of the codes are well explained by its comments
This is how the chaser can automatically run:
```
class ManualMover(turtle.RawTurtle):
    def __init__(self, canvas, step_move=10, step_turn=10):
        super().__init__(canvas)
        self.step_move = step_move
        self.step_turn = step_turn

        # Register event handlers
        canvas.onkeypress(lambda: self.forward(self.step_move), 'Up')
        canvas.onkeypress(lambda: self.backward(self.step_move), 'Down')
        canvas.onkeypress(lambda: self.left(self.step_turn), 'Left')
        canvas.onkeypress(lambda: self.right(self.step_turn), 'Right')
        canvas.listen()

    def run_ai(self):
        pass

    def auto_chase(self, son, mom):
        self.son = son
        self.mom = mom
        x_best = self.pos()[0]
        y_best = self.pos()[1]
        best_dist = 10000000
        x_tmp = x_best
        y_tmp = y_best
        for i in [-10, 0, 10]:
            for j in [-10, 0, 10]:
                x = self.pos()[0] + i
                y = self.pos()[1] + j
                ok = True
                if distance(x, y, self.son.pos()[0], self.son.pos()[1]) > best_dist:
                    ok = False
                if distance(x, y, self.mom.pos()[0], self.mom.pos()[1]) <= 50**2:
                    ok = False
                elif abs(x) < 465 and abs(y) < 395:
                    x_tmp = x
                    y_tmp = y
                if abs(x) > 465 or abs(y) > 395:
                    ok = False
                if ok:
                    best_dist = distance(x, y, self.son.pos()[0], self.son.pos()[1])
                    x_best = x
                    y_best = y

        if x_best != self.pos()[0] or y_best != self.pos()[1]:
            self.goto(x_best, y_best)
        else:
            self.goto(x_tmp, y_tmp)
```
This code brute forces 8 possible moves and chooses the best one. That is, the move get closest to the runner but also not caught by the runner's mom. There are some special numbers like 395, 465. In my local computer, the turtle graphic is displayed with the width from -465 to 465 and the height from -395 to 395. This code also prevents the cases when some turtle is out of visible screen.

The below is how the runner and his mom move:
```
class RandomMover(turtle.RawTurtle):
    def __init__(self, canvas, step_move=10, step_turn=10):
        super().__init__(canvas)
        self.step_move = step_move
        self.step_turn = step_turn

    def run_ai(self, opponent):
        mode = random.randint(0, 1)
        if mode == 0:
            self.left(self.step_turn)
            self.forward(self.step_move)
        elif mode == 1:
            self.right(self.step_turn)
            self.forward(self.step_move)

        self.exceeded_margin_check()

    def mom_move(self, son):
        self.son = son
        x_mom = self.pos()[0]
        y_mom = self.pos()[1]
        x_runner = self.son.pos()[0]
        y_runner = self.son.pos()[1]
        if abs(x_mom - x_runner) < 50 and abs(y_mom - y_runner) < 50:
            mode = random.randint(0, 1)
            if mode == 0:
                self.left(self.step_turn)
                self.forward(self.step_move)
            elif mode == 1:
                self.right(self.step_turn)
                self.forward(self.step_move)

            self.exceeded_margin_check()
            return
        if x_mom < x_runner + 50:
            x_mom += 10
        else:
            x_mom -= 10
        if y_mom < y_runner + 50:
            y_mom += 10
        else:
            y_mom -= 10
        self.goto(x_mom, y_mom)
        self.exceeded_margin_check()


    def exceeded_margin_check(self):
        x = self.pos()[0]
        y = self.pos()[1]
        if abs(x) > 465:
            if x > 0:
                x = min(x, 465)
            else:
                x = max(x, -465)
        if abs(y) > 395:
            if y > 0:
                y = min(y, 395)
            else:
                y = max(y, -395)
        self.goto(x, y)

```
The **run_ai()** function simply makes a random move (does not sound like its name)
The **mom_move()** function helps the mom to find his son quickly. When the distance between them is very far, it chooses the best move to reduce the distance. When they get closer, the mom simply moves randomly around her son.
The **exceeded_margin_check()** function checks whether some turtle has been out of the screen or not, then fixed their move.
