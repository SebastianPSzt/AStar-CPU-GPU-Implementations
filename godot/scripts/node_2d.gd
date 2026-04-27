extends Node2D

var width = 2014
var height = 1024
var perlin_scale = 0.75
var threshold = 0.15
var cell_size = 1
var this_seed = randi()
var save_path = "C:/Users/thysv/source/AStar-CPU-GPU-Implementations/godot/grids/region.json"

var noise = FastNoiseLite.new()
var grid = []

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
		"grid": []
	}

	for y in range(height):
		var row = []
		for x in range(width):
			row.append(grid[y][x]["noise"])
		save_data["grid"].append(row)

	var file = FileAccess.open(path, FileAccess.WRITE)
	if file == null:
		print("Failed to open file for writing: ", path)
		return

	file.store_string(JSON.stringify(save_data, "\t"))
	file.close()
	print("Saved JSON to ", path)

func _ready() -> void:
	noise.noise_type = FastNoiseLite.TYPE_PERLIN
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	
	setup_camera()

	generate_grid()
	build_grid_texture()
	save_noise_to_json(save_path)
	queue_redraw()
	
	var abs_path_region = ProjectSettings.globalize_path("res://grids/region.json")
	var abs_path_result = ProjectSettings.globalize_path("res://grids/result.txt")
	
	var wrapper = RunWrapper.new()
	add_child(wrapper)
	wrapper.run(abs_path_region, abs_path_result)
	
	load_result_from_txt(abs_path_result)
	build_explored_texture()
	visible_path_count = 0
	queue_redraw()

func _draw():
	var draw_size = Rect2(0, 0, width * cell_size, height * cell_size)

	if grid_texture:
		draw_texture_rect(grid_texture, draw_size, false)

	if explored_texture:
		draw_texture_rect(explored_texture, draw_size, false)

	for i in range(visible_path_count):
		var pos = path_order[i]
		draw_rect(
			Rect2(pos.x * cell_size, pos.y * cell_size, cell_size, cell_size),
			Color.RED
		)

func _process(_delta):
	if visible_path_count < path_order.size():
		visible_path_count = min(
			visible_path_count + cells_per_frame,
			path_order.size()
		)
		queue_redraw()

var path_cells = {}
var path_order = []
var visible_path_count = 0
var path_delay = 0.001
var explored_cells = {}
var grid_texture: ImageTexture
var explored_texture: ImageTexture
var cells_per_frame = 100

func load_result_from_txt(path: String):
	explored_cells.clear()
	path_order.clear()
	visible_path_count = 0

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

func animate_path():
	visible_path_count = 0

	while visible_path_count < path_order.size():
		visible_path_count += 1
		queue_redraw()
		await get_tree().create_timer(path_delay).timeout

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
	img.fill(Color(0, 0, 0, 0)) # transparent

	for pos in explored_cells.keys():
		img.set_pixel(pos.x, pos.y, Color(1.0, 0.75, 0.25, 0.65))

	explored_texture = ImageTexture.create_from_image(img)
	
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
