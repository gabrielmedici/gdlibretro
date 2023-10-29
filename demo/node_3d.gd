extends Node2D

@onready var tex_rect = get_node("TextureRect")

# Called when the node enters the scene tree for the first time.
func _ready():
	RetroHost.load_core("vice_x128_libretro")
	#LibRetroHost.load_core("D:\\Documents\\Code\\noarch\\demo\\bin\\LibRetroHost\\lib\\Windows-AMD64\\LibRetroHost-d.dll")


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	RetroHost.run()
	var frame_buffer = RetroHost.get_frame_buffer()
	if(!frame_buffer):
		return
		
	var img_tex = ImageTexture.create_from_image(frame_buffer)
	tex_rect.texture = img_tex
	return
