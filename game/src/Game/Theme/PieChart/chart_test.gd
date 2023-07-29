extends Control



# Called when the node enters the scene tree for the first time.
func _ready():
	var chart:PieChart = $LayeredChart/OuterPieChart
	chart.addOrReplaceLabel("label4",3,"Socialist",Color(1.0,0.0,0.0))

	var chart2:PieChart = $PieChart3
	chart2.RemoveLabel("label1")
	chart2.RemoveLabel("label2")
	chart2.RemoveLabel("label3")
	chart2.RemoveLabel("label4")
	
	#$PieChart
	#$PieChart2 
	#$PieChart/PieChart2 
	var chart3:PieChart = $LayeredChart/InnerPieChart
	chart3.RemoveLabel("label1")
	chart3.RemoveLabel("label2")
	chart3.RemoveLabel("label3")
	chart3.addOrReplaceLabel("a",3,"",Color(1.0,0.0,0.0))
	chart3.addOrReplaceLabel("b",3,"Y",Color(0.0,1.0,0.0))
	chart3.addOrReplaceLabel("c",3,"Z",Color(0.0,0.0,1.0))
	
	
	var chart4:PieChart = $PieChart4
	chart4.addOrReplaceLabel("hi",3,"antidisenstablishmentarianist antidisenstablishmentarianism",Color(0.0,0.5,0.5))
# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
