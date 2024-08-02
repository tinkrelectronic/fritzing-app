# Copyright (c) 2021 Fritzing GmbH

message("Using fritzing svgpp detect script.")

exists($$absolute_path($$PWD/../../svgpp)) {
			SVGPPPATH = $$absolute_path($$PWD/../../svgpp)
            message("found svgpp in $${SVGPPPATH}")
        }

message("including $$absolute_path($${SVGPPPATH}/include)")
INCLUDEPATH += $$absolute_path($${SVGPPPATH}/include)
