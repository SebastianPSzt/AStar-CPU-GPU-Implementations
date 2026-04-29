extends Node2D

#var width = 128
#var height = 128
#var cell_size = 8

var width = 1024
var height = 1024
var cell_size = 1

var perlin_scale = 1.5
var threshold = 0.1
#var this_seed = 3842849903
var this_seed = randi()

#var wrapper = RunCPUWrapper.new()
var wrapper = RunGPU4Wrapper.new()

var save_path = "C:/Users/thysv/source/AStar-CPU-GPU-Implementations/godot/grids/region.json"

var noise = FastNoiseLite.new()
var grid = []

var grid_texture: ImageTexture
var explored_texture: ImageTexture
var path_texture: ImageTexture
var path_img: Image

var explored_cells = {}
var path_order = []
var path_index = 0

var frontier_order = []
var frontier_index = 0
var current_green_frontier = []

var frontier_hold_frames = 5
var frontier_frames_left = 0

var last_green_cell: Vector2i = Vector2i(-1, -1)
var green_hold_frames = 5
var green_frames_left = 0

var correct_path_texture: ImageTexture
var correct_path_cells = {}

func _ready() -> void:
	noise.noise_type = FastNoiseLite.TYPE_PERLIN
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST

	setup_camera()

	generate_grid()
	build_grid_texture()
	save_noise_to_json(save_path)

	var abs_path_region = ProjectSettings.globalize_path("res://grids/region.json")
	var abs_path_result = ProjectSettings.globalize_path("res://grids/result.txt")
	
	add_child(wrapper)
	wrapper.run(abs_path_region, abs_path_result)

	load_result_from_txt(abs_path_result)
	build_explored_texture()
	build_path_texture()

	queue_redraw()
	
func build_correct_path_texture():
	var img = Image.create(width, height, false, Image.FORMAT_RGBA8)
	img.fill(Color(0, 0, 0, 0))

	for pos in correct_path_cells.keys():
		img.set_pixel(pos.x, pos.y, Color(0.25, 0.8, 1.0, 0.75))

	correct_path_texture = ImageTexture.create_from_image(img)


func _draw():
	var draw_size = Rect2(0, 0, width * cell_size, height * cell_size)

	if grid_texture:
		draw_texture_rect(grid_texture, draw_size, false)

	if explored_texture:
		draw_texture_rect(explored_texture, draw_size, false)

	if path_texture:
		draw_texture_rect(path_texture, draw_size, false)

	if correct_path_texture:
		draw_texture_rect(correct_path_texture, draw_size, false)

	# draw current frontier on top
	for pos in current_green_frontier:
		draw_rect(
			Rect2(pos.x * cell_size, pos.y * cell_size, cell_size, cell_size),
			Color.GREEN
		)

func _process(_delta):
	if frontier_frames_left > 0:
		frontier_frames_left -= 1
		return

	# Turn previous green frontier red
	for pos in current_green_frontier:
		path_img.set_pixel(pos.x, pos.y, Color.RED)

	current_green_frontier.clear()

	if frontier_index >= frontier_order.size():
		path_texture.update(path_img)
		queue_redraw()
		return

	# Draw next frontier all at once
	var frontier = frontier_order[frontier_index]

	for pos in frontier:
		path_img.set_pixel(pos.x, pos.y, Color.GREEN)
		current_green_frontier.append(pos)

	frontier_index += 1
	frontier_frames_left = frontier_hold_frames

	path_texture.update(path_img)
	queue_redraw()


func generate_grid():
	grid = []
	noise.seed = this_seed

	for y in range(height):
		var row = []
		for x in range(width):
			var n = noise.get_noise_2d(x * perlin_scale, y * perlin_scale)
			var blocked = n > threshold

			row.append({
				"noise": n,
				"blocked": blocked
			})

		grid.append(row)


func save_noise_to_json(path: String):
	var save_data = {
		"seed": this_seed,
		"width": width,
		"height": height,
		"perlin_scale": perlin_scale,
		"threshold": threshold,
		"cell_size": cell_size,
		"_grid": []
	}

	for y in range(height):
		var row = []
		for x in range(width):
			row.append(grid[y][x]["noise"])
		save_data["_grid"].append(row)

	var file = FileAccess.open(path, FileAccess.WRITE)
	if file == null:
		print("Failed to open file for writing: ", path)
		return

	file.store_string(JSON.stringify(save_data, "\t"))
	file.close()
	print("Saved JSON to ", path)


func load_result_from_txt(path: String):
	explored_cells.clear()
	path_order.clear()
	frontier_order.clear()
	frontier_index = 0
	current_green_frontier.clear()

	var file = FileAccess.open(path, FileAccess.READ)
	if file == null:
		print("Failed to open result file: ", path)
		return

	# --- FIRST LINE: CORRECT PATH ---
	if not file.eof_reached():
		var line = file.get_line().strip_edges()
		if line != "":
			var parts = line.split(",")

			for part in parts:
				var id = part.to_int()
				var x = id % width
				var y = int(id / width)
				var pos = Vector2i(x, y)

				if x >= 0 and x < width and y >= 0 and y < height:
					correct_path_cells[pos] = true

	# # --- REMAINING LINES: HISTORY ---
	while not file.eof_reached():
		var line = file.get_line().strip_edges()
		if line == "":
			continue

		var parts = line.split(",")
		var frontier = []

		for part in parts:
			var id = part.to_int()
			var x = id % width
			var y = int(id / width)
			var pos = Vector2i(x, y)

			if x >= 0 and x < width and y >= 0 and y < height:
				explored_cells[pos] = true
				frontier.append(pos)

		if frontier.size() > 0:
			frontier_order.append(frontier)

	file.close()

	# Build correct path texture AFTER loading
	build_correct_path_texture()


func build_grid_texture():
	var img = Image.create(width, height, false, Image.FORMAT_RGBA8)

	for y in range(height):
		for x in range(width):
			var cell = grid[y][x]
			var color = Color.BLACK if cell["blocked"] else Color.WHITE
			img.set_pixel(x, y, color)

	grid_texture = ImageTexture.create_from_image(img)


func build_explored_texture():
	var img = Image.create(width, height, false, Image.FORMAT_RGBA8)
	img.fill(Color(0, 0, 0, 0))

	for pos in explored_cells.keys():
		img.set_pixel(pos.x, pos.y, Color(1.0, 0.75, 0.25, 0.65))

	explored_texture = ImageTexture.create_from_image(img)


func build_path_texture():
	path_img = Image.create(width, height, false, Image.FORMAT_RGBA8)
	path_img.fill(Color(0, 0, 0, 0))

	path_texture = ImageTexture.create_from_image(path_img)
	frontier_index = 0
	current_green_frontier.clear()


func setup_camera():
	var cam = Camera2D.new()
	add_child(cam)
	cam.make_current()

	var grid_size = Vector2(width * cell_size, height * cell_size)
	var viewport_size = get_viewport_rect().size

	cam.position = grid_size / 2

	var zoom_x = grid_size.x / viewport_size.x
	var zoom_y = grid_size.y / viewport_size.y
	var zoom = max(zoom_x, zoom_y)

	cam.zoom = Vector2(zoom, zoom)
