extends Node2D

var width = 576
var height = 324
var perlin_scale = 0.75
var threshold = 0
var cell_size = 2
var seed = randi()
var save_path = "C:/Users/thysv/source/AStar-CPU-GPU-Implementations/godot/grids/region.json"

var noise = FastNoiseLite.new()
var grid = []

func generate_grid():
	grid = []
	noise.seed = seed

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
		"seed": seed,
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
	generate_grid()
	save_noise_to_json(save_path)
	queue_redraw()

func _draw():
	for y in range(height):
		for x in range(width):
			var cell = grid[y][x]
			var color = Color.BLACK if cell["blocked"] else Color.WHITE

			draw_rect(
				Rect2(x * cell_size, y * cell_size, cell_size, cell_size),
				color
			)
