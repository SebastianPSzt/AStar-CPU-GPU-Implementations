extends Node2D

var width = 128
var height = 128
var perlin_scale = 0.75
var threshold = 0.15
var cell_size = 8
var this_seed = 2484144148

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
var cells_per_frame = 1

var last_green_cell: Vector2i = Vector2i(-1, -1)
var green_hold_frames = 5
var green_frames_left = 0

func _ready() -> void:
	noise.noise_type = FastNoiseLite.TYPE_PERLIN
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST

	setup_camera()

	generate_grid()
	build_grid_texture()
	save_noise_to_json(save_path)

	var abs_path_region = ProjectSettings.globalize_path("res://grids/region.json")
	var abs_path_result = ProjectSettings.globalize_path("res://grids/result.txt")

	var wrapper = RunWrapper.new()
	add_child(wrapper)
	wrapper.run(abs_path_region, abs_path_result)

	load_result_from_txt(abs_path_result)
	build_explored_texture()
	build_path_texture()

	queue_redraw()


func _draw():
	var draw_size = Rect2(0, 0, width * cell_size, height * cell_size)

	if grid_texture:
		draw_texture_rect(grid_texture, draw_size, false)

	if explored_texture:
		draw_texture_rect(explored_texture, draw_size, false)

	if path_texture:
		draw_texture_rect(path_texture, draw_size, false)

func _process(_delta):
	if green_frames_left > 0:
		green_frames_left -= 1
		return

	if last_green_cell.x != -1:
		path_img.set_pixel(last_green_cell.x, last_green_cell.y, Color.RED)

	if path_index >= path_order.size():
		path_texture.update(path_img)
		queue_redraw()
		return

	var pos = path_order[path_index]
	path_img.set_pixel(pos.x, pos.y, Color.GREEN)

	last_green_cell = pos
	path_index += 1
	green_frames_left = green_hold_frames

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
	path_index = 0

	var file = FileAccess.open(path, FileAccess.READ)
	if file == null:
		print("Failed to open result file: ", path)
		return

	while not file.eof_reached():
		var line = file.get_line().strip_edges()
		if line == "":
			continue

		var id = line.to_int()
		var x = id % width
		var y = int(id / width)
		var pos = Vector2i(x, y)

		if x >= 0 and x < width and y >= 0 and y < height:
			explored_cells[pos] = true
			path_order.append(pos)

	file.close()


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
	path_index = 0


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
