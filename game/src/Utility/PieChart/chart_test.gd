extends Control



# Called when the node enters the scene tree for the first time.
func _ready():
	var chart:PieChart = $PieChart
	chart.addOrReplaceLabel("label4",3,"Socialist",Color(1.0,0.0,0.0))

	var chart2:PieChart = $PieChart2
	chart2.RemoveLabel("label1")
	chart2.RemoveLabel("label2")
	chart2.RemoveLabel("label3")
	chart2.RemoveLabel("label4")
	
	var chart3:PieChart = $PieChart3
	chart3.RemoveLabel("label1")
	chart3.RemoveLabel("label2")
	chart3.RemoveLabel("label3")
	chart3.addOrReplaceLabel("a",3,"X",Color(1.0,0.0,0.0))
	chart3.addOrReplaceLabel("b",3,"Y",Color(0.0,1.0,0.0))
	chart3.addOrReplaceLabel("c",3,"Z",Color(0.0,0.0,1.0))
# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
