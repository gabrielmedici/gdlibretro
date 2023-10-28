extends Node3D


# Called when the node enters the scene tree for the first time.
func _ready():
	#LibRetroHost.load_core("D:\\Documents\\Code\\noarch\\demo\\vice_x64_libretro.dll")
	LibRetroHost.load_core("D:\\Documents\\Code\\noarch\\demo\\bin\\LibRetroHost\\lib\\Windows-AMD64\\LibRetroHost-d.dll")


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
