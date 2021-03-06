import pygame
import sys
import random
import copy
import time
import json
from BaseClass import *
import numpy as np

scale = 2.0
sleepp = 1 / 20.
score = [0, 0]
deli  = [0, 0]
bonu  = [0, 0]

def PlayJsonFile(mydir):
	print()
	print("Load Replay File={}".format(mydir))

	with open(mydir, "r")as file:
		Json = file.read()
		Json = json.loads(Json)
	init_info = Json[0]
	final_info = Json[-1]
	Json = Json[1:-1]
	MapID = init_info["map"]
	map_dir = "." + os.sep + "Maps" + os.sep
	map_names = os.listdir(map_dir)
	map_names.sort()
	map_name = map_names[MapID]
	with open(map_dir + map_name, "r")as file:
		JSON = file.read()
		JSON = json.loads(JSON)
	width = JSON["width"]
	height = JSON["height"]
	faction_number = JSON["faction_number"]
	birth_places = JSON["birth_places"]
	ball_places = JSON["ball_places"]
	target_places = JSON["target_places"]
	bonus_places = JSON["bonus_places"]
	walls = JSON["walls"]
	walls = np.asarray(walls).astype(np.bool)
	human_number = JSON["human_number"]

	pygame.init()
	screen = pygame.display.set_mode((int(width * scale), int(height * scale)))
	pygame.display.set_caption("Defense Of The sOgou")

	def draw_wall(x, y):
		pygame.draw.rect(screen, black, pygame.Rect(x * room_size * scale, y * room_size * scale,
													room_size * scale, room_size * scale))
	def draw_road(x, y):
		pygame.draw.rect(screen, gray, pygame.Rect(x * room_size * scale, y * room_size * scale,
													room_size * scale, room_size * scale))
	screen.fill(gray)
	for x in range(int(width / room_size)):
		for y in range(int(height / room_size)):
			if walls[x][y]:
				draw_wall(x, y)

	def draw_ball(ball):
		pygame.draw.circle(screen, black, (int(ball.pos.x * scale),
										   int(ball.pos.y * scale)), int(ball.radius * scale))

	def draw_fireball(fireball):
		pygame.draw.circle(screen, (200, 200, 100), (int(fireball.pos.x * scale),
										 int(fireball.pos.y * scale)), int(fireball.radius * scale))

	def draw_target(target):
		pygame.draw.circle(screen, white, (int(target.pos.x * scale),
										  int(target.pos.y * scale)), int(target.radius * scale))

	def draw_meteor(meteor):
		pygame.draw.circle(screen, pink, (int(meteor.pos.x * scale), int(
			meteor.pos.y * scale)), int(meteor.attack_radius * scale))

	def draw_human(human):
		if human.inv_time > 0:
			pygame.draw.circle(screen, golden, (int(human.pos.x * scale), int(
				human.pos.y * scale)), int((fireball_radius) * scale) + 3)
		if human.faction == 0:
			pygame.draw.circle(screen, (100, 233, 233), (int(human.pos.x * scale),
											  int(human.pos.y * scale)), int(fireball_radius * scale + 1))
		else:
			pygame.draw.circle(screen, (233, 100, 233), (int(human.pos.x * scale),
											  int(human.pos.y * scale)), int(fireball_radius * scale + 1))
		myfont = pygame.font.Font(None, 20)
		textImage = myfont.render(str(human.hp)+':'+('%.2f' % (human.pos.x))+','+('%.2f' % (human.pos.y)), True, blue)
		screen.blit(textImage, (human.pos.x * scale, human.pos.y * scale))
		textImage = myfont.render(str(human.number), True, black)
		screen.blit(textImage, ((human.pos.x - 5)* scale, (human.pos.y - 5) * scale))

	def draw_bonus(bonus):
		pygame.draw.circle(screen, yellow, (int(bonus.pos.x * scale),
											int(bonus.pos.y * scale)), int(bonus.radius * scale))
											
											
	def rewrite(u, v, q, w, r = 5):
		for x in range(int(u - r), int(min(u + q, width / room_size))):
			for y in range(int(v - r), int(min(v + w, height / room_size))):
				if walls[x][y]:
			 		draw_wall(x, y)
				else:
			 		draw_road(x, y)							  
											
	def redraw_ball(fireball):
		rewrite(fireball.pos.x, fireball.pos.y, 5, 5, 5)

	def redraw_fireball(fireball):
		rewrite(fireball.pos.x, fireball.pos.y, 5, 5, 5)

	def redraw_target(target):
		rewrite(target.pos.x, target.pos.y, 5, 5, 5)

	def redraw_meteor(meteor):
		rewrite(meteor.pos.x, meteor.pos.y, 5, 5, 5)
		
	
		

	def redraw_human(human):
		if human.inv_time > 0:
			pygame.draw.circle(screen, white, (int(human.pos.x * scale), int(
				human.pos.y * scale)), int((fireball_radius * scale) + 3))
		pygame.draw.circle(screen, white, (int(human.pos.x * scale),
										  int(human.pos.y * scale)), int(fireball_radius * scale))
		# myfont = pygame.font.Font(None, 30)
		# textImage = myfont.render(str(human.hp), True, white)
		# screen.blit(textImage, (human.pos.x * scale, human.pos.y * scale))
		rewrite(human.pos.x, human.pos.y, 60, 8)
		
	def redraw_bonus(bonus):
		pygame.draw.circle(screen, white, (int(bonus.pos.x * scale),
											int(bonus.pos.y * scale)), int(bonus.radius * scale))

	def draw_all(humans, walls, balls, fireballs, meteors, targets, bonuses, score, timecnt):
		for event in pygame.event.get():
			if event.type == pygame.QUIT:
				sys.exit()
		for target in targets:
			draw_target(target)
		for human in humans:
			if human.death_time == -1:
				draw_human(human)
		for fireball in fireballs:
			draw_fireball(fireball)
		for ball in balls:
			draw_ball(ball)
		for bonus in bonuses:
			if bonus.time == -1:
				draw_bonus(bonus)
		for meteor in meteors:
		    draw_meteor(meteor)
		

		for i in (0, 1):
			myfont = pygame.font.Font(None, 60)
			color = (100, 100, 255)
			if i == 1:
				color = (200, 100, 200)
			textImage = myfont.render(str(int(score[i])), True, color)
			screen.blit(textImage, (i * 80 * scale + 80  * scale, 0))
		for i in (0, 1):
			myfont = pygame.font.Font(None, 40)
			textImage = myfont.render(str(int(deli[i])), True, white)
			screen.blit(textImage, (i * 80 * scale + 120 * scale, 0))
		for i in (0, 1):
			myfont = pygame.font.Font(None, 40)
			textImage = myfont.render(str(int(bonu[i])), True, gray)
			screen.blit(textImage, (i * 80 * scale + 140 * scale, 0))
			
		screen.blit(pygame.font.Font(None, 40).render(str(int(timecnt)), True, white), (0, 0))
		pygame.display.flip()
		
		screen.blit(pygame.font.Font(None, 40).render(str(int(timecnt)), True, black), (0, 0))
		
		for i in (0, 1):
			myfont = pygame.font.Font(None, 60)
			textImage = myfont.render(str(int(score[i])), True, black)
			screen.blit(textImage, (i * 80 * scale + 80 * scale, 0))
		for i in (0, 1):
			myfont = pygame.font.Font(None, 40)
			textImage = myfont.render(str(int(deli[i])), True, black)
			screen.blit(textImage, (i * 80 * scale + 120 * scale, 0))
		for i in (0, 1):
			myfont = pygame.font.Font(None, 40)
			textImage = myfont.render(str(int(bonu[i])), True, black)
			screen.blit(textImage, (i * 80 * scale + 140 * scale, 0))
		for target in targets:
			redraw_target(target)
		for meteor in meteors:
			redraw_meteor(meteor)
		for fireball in fireballs:
			redraw_fireball(fireball)
		for ball in balls:
			redraw_ball(ball)
		for human in humans:
			if human.death_time == -1:
				redraw_human(human)
		for bonus in bonuses:
			if bonus.time == -1:
				redraw_bonus(bonus)
				

	targets = [TargetArea(Point(t[0], t[1])) for t in target_places]
	bonuses = [Bonus(num, Point(t[0], t[1]))
			   for num, t in enumerate(bonus_places)]
	humans = [None] * faction_number * human_number
	balls = [None] * faction_number

	for fac, birth_place in enumerate(birth_places):
		for i, pos in enumerate(birth_place):
			humans[i * faction_number +
				   fac] = Human(i * faction_number + fac, Point(pos[0], pos[1]))
			humans[i * faction_number + fac].reset()
	for fac, ball_place in enumerate(ball_places):
		balls[fac] = Ball(Point(ball_place[0], ball_place[1]), fac)
		balls[fac].reset()

	for Data in Json:
		timecnt = Data['frame']
		hs = json.loads(Data["humans"])
		for i in range(faction_number * human_number):
			num = hs[i][0]
			h = humans[i]
			h.pos = Point(hs[i][1], hs[i][2])
			h.hp, h.meteor_num, h.meteor_time, h.flash_num, h.flash_time, h.fireball_time, h.death_time, h.inv_time = tuple(
				hs[i][3:])

		meteors = json.loads(Data["meteors"])
		num_of_meteors = len(meteors)
		for i in range(num_of_meteors):
			pos = Point(meteors[i][0], meteors[i][1])
			lasttime = meteors[i][2]
			from_number = meteors[i][3]
			meteors[i] = Meteor(pos, from_number, meteors[i][4])
			meteors[i].time = lasttime

		b = json.loads(Data["bonus"])
		for i, bb in enumerate(b):
			if bb == 1:
				bonuses[i].time = -1
			else:
				bonuses[i].time = 0

		fireballs = json.loads(Data["fireballs"])
		num_of_fireballs = len(fireballs)
		for i in range(num_of_fireballs):
			pos = Point(fireballs[i][0], fireballs[i][1])
			rot = fireballs[i][2]
			from_number = fireballs[i][3]
			fireballs[i] = Fireball(pos, rot, from_number, fireballs[i][4])

		bs = json.loads(Data["balls"])
		for i in range(faction_number):
			pos = Point(bs[i][0], bs[i][1])
			balls[i].pos = pos
			balls[i].belong = bs[i][2]
			balls[i].faction = bs[i][3]

		events = json.loads(Data["events"])
		if len(events) > 0:
			print(
				"=====================time = {} frames=====================".format(timecnt))

		for event in events:
			if event[0] == 1:
				print("Player {} shoots!".format(event[1]))
			elif event[0] == 2:
				print("Player {} gets {} hurt from {}!".format(
					event[1], event[2], event[3]))
			elif event[0] == 3:
				print("Player {} died at ({},{})!".format(
					event[1], event[2], event[3]))
				score[int(event[1] + 1) % 2] += 1
			elif event[0] == 4:
				print("Player {} cast Meteor!".format(
					event[1]))
			elif event[0] == 5:
				print("Player {} gets ball!".format(
					event[1]))
			elif event[0] == 6:
				print("A Fireball from {} splashes at ({},{})!".format(
					event[3], event[1], event[2]))
			elif event[0] == 7:
				print("A Meteor from {} impacts at ({},{})!".format(
					event[3], event[1], event[2]))
			elif event[0] == 8:
				print("Player {} reincarnate!".format(
					event[1]))
			elif event[0] == 9:
				print("Player {} flashes from ({},{}) to ({},{})!".format(
					event[1], event[2], event[3], event[4], event[5]))
			elif event[0] == 10:
				print("Player {} goals with faction{}'s ball!".format(
					event[1], event[2]))
				score[int(event[1]) % 2] += 80
				deli[int(event[1]) % 2] += 1
			elif event[0] == 11:
				print("Bonus_place {} generate a bonus!".format(event[1]))
			elif event[0] == 12:
				print("Player {} get a bonus at bonus_place {}".format(
					event[1], event[2]))
				score[int(event[1]) % 2] += 10
				bonu[int(event[1]) % 2] += 1
		draw_all(humans, walls, balls, fireballs, meteors, targets, bonuses, score, timecnt)
		time.sleep(sleepp)

	print("=====================score board=====================")

	for i, s in enumerate(json.loads(final_info["scores"])):
		print("Player{} :".format(i), s)
	print("=====================================================")


if __name__ == "__main__":
	if len(sys.argv) == 2:
		PlayJsonFile(sys.argv[1])
	else:
		replay_dir = "." + os.sep + "Replay" + os.sep
		replay_names = os.listdir(replay_dir)
		for replay_name in replay_names:
			if not replay_name.endswith(".json"):
				replay_names.remove(replay_name)
		if len(replay_names) == 0:
			raise Exception("No Replay File in default dir")

		def TimeStampToTime(timestamp):
			timeStruct = time.localtime(timestamp)
			return timeStruct

		def get_FileModifyTime(filePath):
			t = os.path.getmtime(filePath)
			return TimeStampToTime(t)

		newest_t = get_FileModifyTime(replay_dir + replay_names[0])
		name = replay_names[0]

		for replay_name in replay_names[1:]:
			T = get_FileModifyTime(replay_dir + replay_name)
			if newest_t < T:
				newest_t = T
				name = replay_name
		PlayJsonFile(replay_dir + name)
