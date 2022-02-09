## Video Link

https://youtu.be/0i97YPlt3I0

## Hot keys

1 - test mode

2 - level 1

3 - level 2

F1 - replay

F3 - back to main menu

G - use gravity

B - use quad tree

N - level 2 show mesh board

Arrows – control level 2 green ball (player)

## Play

### Main Menu
![](https://s3.bmp.ovh/imgs/2022/02/072bdc93d769494a.png)
 
### Test Mode

There some basic object to show collision effect.
![](https://s3.bmp.ovh/imgs/2022/02/ff3e82d2c679f79c.jpg)
 
### Level 1
![](https://s3.bmp.ovh/imgs/2022/02/176b4eb025eb3849.png)
Ball generate at bottom left，green block will give ball a force to up forward (both linear speed and angular speed). And ball will move with yellow platform, then click the purple plane, It will hit the ball to make it fall to ramp.
 
 ![](https://s3.bmp.ovh/imgs/2022/02/4ca7156349635165.png)

On the first ramp, there three coin, ball have collision with them will get 10 score each

 ![](https://s3.bmp.ovh/imgs/2022/02/88670667cfcb305f.png)

The second ramp have resilient surface, and click purple capsule can hit the ball to the right. When ball at a suitable position, click the pillar make the second ramp rotate, then the ball will roll down.

If it have collision with the end green block in limit time, game win. Otherwise game lose.
And when win a game, screen will show time.

![](https://s3.bmp.ovh/imgs/2022/02/e5a116901f1716b0.jpg)

### Level 2

On the top left there are show 3 kinds length of path-finding algorithm result and enemy current speed, and enemy (red ball) will choose the shortest one to move.

Enemy use behaviour-tree, and will choose one in 3 behaviours ( seek bonus, seek player, seek speed up block).

When enemy change strategy, it will have ray shoot to coin (ray cast), if there is no other obstacles. It will seek coin.

And AI also find player’s position each frame.

 ![](https://s3.bmp.ovh/imgs/2022/02/24160c8102671828.png)

 ![](https://s3.bmp.ovh/imgs/2022/02/c7af0ab5b5c4f83a.png)

When player have collision with blue block on top right, the block with same color on the way to end point(red block) will disappear. Then hit the red block will win this game.
But when enemy hit the player, they will both respawn at initial position and reset game.

![](https://s3.bmp.ovh/imgs/2022/02/3f5243f3d171906f.png)
 
And press N in level 2, screen will show mesh line of the maze.
 
![](https://s3.bmp.ovh/imgs/2022/02/9e23b1fd373129dd.png)