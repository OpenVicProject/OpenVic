# Event Design

> **Warning**
> The following document is ***not in a finished state***

# Event Proposal
- Strings are combinations of characters within double quotes `""`
- When a string is prefixed by `L__` it is replaced by the localization text that has the string as a key
- Booleans are either `true` or `false`
- Boolean expressions make use of the logical operators `and`, `or`, and `not`
- `addModifier`: pseudo-function that adds the string as a national modifier
- `hasModifier`: pseudo-function that checks if the nation has the modifier specified by the string; evaluates to `boolean`
- `fireEvent`: pseudo-function which fires the specified event for all nations where the provided boolean expression evaluates to `true`

## Sample Event
```
{
	id = "eventTrianglePentagonDebate"
	imageName = "coolPicture.png"
	title = "What shape is the best?"
	description = L__"SHAPE_DEBATE_DESCRIPTION" //Look up localization text with the string as a key
	////Equivalent to
	//description = "A fierce debate has broken out! Are Triangles the best shape, or Pentagons?"

	eligibility = {
		(literacy >= 70 and not hasModifier("hatesGeometry")) or
		nationId == "GreatBritain" or nationId == "Austria"
	}

	//If eventRepetitionLimit is not specified, it can fire an unlimited number of times
	eventRepetitionLimit = 2 //This event can fire twice

	//If mtth is omitted, then the event can only be triggered by another event or a decision
	mtth = {
		months = 36
		modifiers = [
			{
				factor = 1 / averageMilitancy
				when = { averageMilitancy > 4}
			},
			{
				factor = 1.2
				when = { averageMilitancy <= 2 or hasModifier("geometricPoliceActions") }
			},
			{
				factor = 2.5
				when = { hasModifier("dislikesGeometry") }
			}
		]
	}

	options = [
		{
			text = "Triangles, obviously!"
			effect = { //Effects are made with reference to the nation which had the event
				prestige += 5
				addModifier("supportForTriangles")
			}
		},
		{
			text = "Pentagons are superior!"
			effect = {
				prestige += 5
				addModifier("supportForPentagons")
			}
			secretEffect = {
				addModifier("secretSocietyOfTriangleists")
			}
		},
		{
			text = "No more silliness about these shapes!"
			hoverText = "Stop the debate!"
			effect = {
				if prestige > 50 {
					prestige = 50
				}
				else if prestige < 50 {
					money -= 3000
				}
				else {
					prestige -= 4
				}
				addModifier("hatesGeometry")
				fireEvent("eventNoMoreShapes", nationId != this(nationId) and not hasModifier("hatesGeometry"))
			}
		}
	]
}
```

### Event Attributes
| Attribute            | Type                       |
|----------------------|----------------------------|
| id                   | string                     |
| imageName            | string                     |
| description          | string                     |
| eligibility          | boolean expression         |
| eventRepetitionLimit | number literal             |
| mtth                 | mean-time-to-happen object |
| options              | array of option objects    |

### Mean-Time-To-Happen Attributes
| Attribute | Type                         |
|-----------|------------------------------|
| months    | number (non-negative)        |
| modifiers | array of mtth factor objects |

### MTTH Factor Attributes
| Attribute | Type               |
|-----------|--------------------|
| factor    | number             |
| when      | boolean expression |

### Option Attributes
| Attribute    | Type   |
|--------------|--------|
| text         | string |
| effect       | Series of statements within Domain Specific Language (DSL) |
| secretEffect | Series of statements within Domain Specific Language (DSL) |