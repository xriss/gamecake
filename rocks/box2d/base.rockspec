build={
 modules={
      ["wetgenes.box2d"]="lua_boxxd/code/box2d.lua",
      ["wetgenes.box2d.core"]={
         sources={
			"lua_boxxd/code/lua_box2d.c",
			"lib_box2d/git/src/aabb.c",
			"lib_box2d/git/src/arena_allocator.c",
			"lib_box2d/git/src/bitset.c",
			"lib_box2d/git/src/body.c",
			"lib_box2d/git/src/broad_phase.c",
			"lib_box2d/git/src/constraint_graph.c",
			"lib_box2d/git/src/contact.c",
			"lib_box2d/git/src/contact_solver.c",
			"lib_box2d/git/src/core.c",
			"lib_box2d/git/src/distance.c",
			"lib_box2d/git/src/distance_joint.c",
			"lib_box2d/git/src/dynamic_tree.c",
			"lib_box2d/git/src/geometry.c",
			"lib_box2d/git/src/hull.c",
			"lib_box2d/git/src/id_pool.c",
			"lib_box2d/git/src/island.c",
			"lib_box2d/git/src/joint.c",
			"lib_box2d/git/src/manifold.c",
			"lib_box2d/git/src/math_functions.c",
			"lib_box2d/git/src/motor_joint.c",
			"lib_box2d/git/src/mover.c",
			"lib_box2d/git/src/parallel_for.c",
			"lib_box2d/git/src/physics_world.c",
			"lib_box2d/git/src/prismatic_joint.c",
			"lib_box2d/git/src/recording.c",
			"lib_box2d/git/src/recording_replay.c",
			"lib_box2d/git/src/revolute_joint.c",
			"lib_box2d/git/src/scheduler.c",
			"lib_box2d/git/src/sensor.c",
			"lib_box2d/git/src/shape.c",
			"lib_box2d/git/src/solver.c",
			"lib_box2d/git/src/solver_set.c",
			"lib_box2d/git/src/table.c",
			"lib_box2d/git/src/timer.c",
			"lib_box2d/git/src/types.c",
			"lib_box2d/git/src/weld_joint.c",
			"lib_box2d/git/src/wheel_joint.c",
			"lib_box2d/git/src/world_snapshot.c",
         },
         incdirs={
            "lua_box2d",
            "lib_box2d/git/include",
         },
      },
 },
 platform={
  windows={
  },
 },
 type="builtin",
}
dependencies={
 "lua >= 5.1 <= 5.2",
}
description={
 homepage="https://github.com/xriss/gamecake",
 license="MIT",
 summary="box2d V3",
 detailed=[[

A lua binding to Box2d V3 physics library.

See libs/lua_boxxd/readme.md in the gamecake repo 
https://github.com/xriss/gamecake/blob/master/libs/lua_boxxd/

 ]],
}
