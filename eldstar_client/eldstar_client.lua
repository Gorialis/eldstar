
local socket = require("socket")


-- memory locations
romhash = gameinfo.getromhash()

if romhash == "B9CCA3FF260B9FF427D981626B82F96DE73586D3" then  -- Japan (JP, NTSC)
    useragent = "BizHawk - Mario Story (J)"

    collider_summary_struct = 0x0B42C0
    collider_table_ptr = collider_summary_struct + 4
    collider_amount_ptr = collider_summary_struct + 12

    world_object_table_ptr = 0x1564B0
    dynamic_object_table_ptr = 0x09E9A0
    item_table_ptr = 0x15B520

    mario_x_ptr = 0x10F1B0
    mario_y_ptr = 0x10F1B4
    mario_z_ptr = 0x10F1B8
    mario_rot_ptr = 0x10F230

    camera_data_ptr = 0x09E6D0
    camera_side_pan = 0x0B226C

    camera_deferred = 0x0B1D9C
elseif romhash == "2111D39265A317414D359E35A7D971C4DFA5F9E1" then  -- Europe (EU, PAL)
    useragent = "BizHawk - Paper Mario (E)"

    collider_summary_struct = 0x0B29C0
    collider_table_ptr = collider_summary_struct + 4
    collider_amount_ptr = collider_summary_struct + 12

    world_object_table_ptr = 0x150030
    dynamic_object_table_ptr = 0x09F300
    item_table_ptr = 0x155160

    mario_x_ptr = 0x10DAF0
    mario_y_ptr = 0x10DAF4
    mario_z_ptr = 0x10DAF8
    mario_rot_ptr = 0x10DB70

    camera_data_ptr = 0x09F020
    camera_side_pan = 0x0B096C

    camera_deferred = 0x0B049C
else -- United States (US, NTSC)
    if romhash ~= "3837F44CDA784B466C9A2D99DF70D77C322B97A0" then  -- This might be a Star Rod mod
        print("I don't know what this ROM is.")
        print("I'm going to assume it's a Star Rod mod and load the US configuration.")
        useragent = "BizHawk - Unknown"
    else
        useragent = "BizHawk - Paper Mario (U)"
    end

    collider_summary_struct = 0x0B42E0
    collider_table_ptr = collider_summary_struct + 4
    collider_amount_ptr = collider_summary_struct + 12

    world_object_table_ptr = 0x151470
    dynamic_object_table_ptr = 0x0A0B90
    item_table_ptr = 0x1565A0

    mario_x_ptr = 0x10EFF0
    mario_y_ptr = 0x10EFF4
    mario_z_ptr = 0x10EFF8
    mario_rot_ptr = 0x10F070

    camera_data_ptr = 0x0A08C0
    camera_side_pan = 0x0B228C

    camera_deferred = 0x0B1DBC
end


-- variables
framebuffer = {}

onlyZoned = false


-- optims
local read_float = mainmemory.readfloat
local read_u32_be = mainmemory.read_u32_be
local read_u16_be = mainmemory.read_u16_be

-- functions
function send_colliders()
    base_address = read_u32_be(collider_table_ptr) - 0x80000000 -- 0x8026A060
    total = read_u16_be(collider_amount_ptr) -- 0x49

    -- prevent breaking if world is not loaded
    if base_address >= 0x200000 and base_address <= 0x2FB800 then
        for collider = 0,total-1 do
            collider_active = bit.band(read_u32_be(base_address + (collider * 0x1C)), 0x10000) == 0 and 1 or 0
            tri_count = read_u16_be(base_address + (collider * 0x1C) + 0x0A)
            tri_table_ptr = read_u32_be(base_address + (collider * 0x1C) + 0x0C) - 0x80000000
            collider_ptr = read_u32_be(base_address + (collider * 0x1C) + 0x10) - 0x80000000

            -- ensure this collider is not badly formed
            if tri_count ~= 0 and tri_table_ptr >= 0x200000 and tri_table_ptr <= 0x2FB800 and collider_ptr >= 0x200000 and collider_ptr <= 0x2FB800 and bit.band(collider_ptr - 0x10, 0x10000) == 0 then
                left = read_float(collider_ptr, true)
                bottom = read_float(collider_ptr + 0x04, true)
                back = read_float(collider_ptr + 0x08, true)
                right = read_float(collider_ptr + 0x0C, true)
                top = read_float(collider_ptr + 0x10, true)
                front = read_float(collider_ptr + 0x14, true)

                if not onlyZoned or (mario_x >= left - 13 and mario_x <= right + 13 and mario_y >= bottom - 18.5 and mario_y <= top and mario_z >= back - 13 and mario_z <= front + 13) then
                    -- g stands for 'geometry'
                    for tri = 0,tri_count-1 do
                        tri_loc = tri_table_ptr + (0x40 * tri)
                        tri_1_ptr = read_u32_be(tri_loc) - 0x80000000
                        tri_2_ptr = read_u32_be(tri_loc + 0x04) - 0x80000000
                        tri_3_ptr = read_u32_be(tri_loc + 0x08) - 0x80000000

                        unit_x = read_float(tri_loc + 0x30, true)
                        unit_y = read_float(tri_loc + 0x34, true)
                        unit_z = read_float(tri_loc + 0x38, true)

                        -- check tris are actually valid
                        if tri_1_ptr >= 0x200000 and tri_1_ptr <= 0x2FB800 and tri_2_ptr >= 0x200000 and tri_2_ptr <= 0x2FB800 and tri_3_ptr >= 0x200000 and tri_3_ptr <= 0x2FB800 then
                            tri_1_x = read_float(tri_1_ptr, true)
                            tri_1_y = read_float(tri_1_ptr + 0x04, true)
                            tri_1_z = read_float(tri_1_ptr + 0x08, true)

                            tri_2_x = read_float(tri_2_ptr, true)
                            tri_2_y = read_float(tri_2_ptr + 0x04, true)
                            tri_2_z = read_float(tri_2_ptr + 0x08, true)

                            tri_3_x = read_float(tri_3_ptr, true)
                            tri_3_y = read_float(tri_3_ptr + 0x04, true)
                            tri_3_z = read_float(tri_3_ptr + 0x08, true)

                            center_x = (tri_1_x + tri_2_x + tri_3_x) / 3
                            center_y = (tri_1_y + tri_2_y + tri_3_y) / 3
                            center_z = (tri_1_z + tri_2_z + tri_3_z) / 3

                            framebuffer[#framebuffer + 1] = string.format("g %02X %f %f %f %f %f %f %f %f %f %f %f %f %d\n", collider, tri_1_x, tri_1_y, tri_1_z, tri_2_x, tri_2_y, tri_2_z, tri_3_x, tri_3_y, tri_3_z, unit_x, unit_y, unit_z, collider_active)
                        end
                    end
                end
            end
        end
    end

    wo_base = read_u32_be(world_object_table_ptr) - 0x80000000 -- 0x801563C0

    -- prevent breaking if world not loaded
    if wo_base >= 0x100000 and wo_base <= 0x1FB800 then
        for wobj = 0,0x1E-1 do
            wo_ptr = read_u32_be(wo_base + (4 * wobj)) - 0x80000000

            -- ignore empty slots
            if wo_ptr >= 0x300000 and wo_ptr <= 0x3FB800 then
                obj_x = read_float(wo_ptr + 0x48, true)
                obj_y = read_float(wo_ptr + 0x4C, true)
                obj_z = read_float(wo_ptr + 0x50, true)

                -- these don't affect collision and are here for documentation purposes only
                --obj_xscale = read_float(wo_ptr + 0x54, true)
                --obj_yscale = read_float(wo_ptr + 0x58, true)
                --obj_zscale = read_float(wo_ptr + 0x5C, true)

                obj_xrot = read_float(wo_ptr + 0x60, true)
                obj_yrot = read_float(wo_ptr + 0x64, true)
                obj_zrot = read_float(wo_ptr + 0x68, true)

                width = read_u16_be(wo_ptr + 0x0C)
                height = read_u16_be(wo_ptr + 0x0E)
                depth = read_u16_be(wo_ptr + 0x10)

                -- w stands for 'world object'
                framebuffer[#framebuffer + 1] = string.format("w %02X %f %f %f %f %f %f %d %d %d\n", wobj, obj_x, obj_y, obj_z, obj_xrot, obj_yrot, obj_zrot, width, height, depth)
            end
        end
    end

    do_base = read_u32_be(dynamic_object_table_ptr) - 0x80000000 -- 0x8009E7A0

    -- prevent breaking if world not loaded
    if do_base >= 0x080000 and do_base <= 0x1FB800 then
        for dobj = 0,0x40-1 do
            do_ptr = read_u32_be(do_base + (4 * dobj)) - 0x80000000

            if do_ptr >= 0x300000 and do_ptr <= 0x3FB800 then
                obj_rot = read_float(do_ptr + 0x34, true)
                obj_x = read_float(do_ptr + 0x38, true)
                obj_y = read_float(do_ptr + 0x3C, true)
                obj_z = read_float(do_ptr + 0x40, true)

                diameter = read_u16_be(do_ptr + 0xA6)
                height = read_u16_be(do_ptr + 0xA8)

                -- d stands for 'dynamic object'
                framebuffer[#framebuffer + 1] = string.format("d %02X %f %f %f %d %d %f\n", dobj, obj_x, obj_y, obj_z, diameter, height, obj_rot)
            end
        end
    end

    item_base = read_u32_be(item_table_ptr) - 0x80000000 -- 0x8015AD20

    -- prevent breaking if world not loaded
    if item_base >= 0x080000 and item_base <= 0x1FB800 then
        for iobj = 0,0x100-1 do
            i_ptr = read_u32_be(item_base + (4 * iobj)) - 0x80000000

            if i_ptr >= 0x300000 and i_ptr <= 0x3FB800 then
                obj_x = read_float(i_ptr + 0x08, true)
                obj_y = read_float(i_ptr + 0x0C, true)
                obj_z = read_float(i_ptr + 0x10, true)

                -- i stands for 'item object'
                framebuffer[#framebuffer + 1] = string.format("i %02X %f %f %f\n", iobj, obj_x, obj_y, obj_z)
            end
        end
    end

    camera_yaw = read_float(camera_data_ptr, true)
    camera_side = read_float(camera_side_pan, true)

    camera_deferred_x = read_float(camera_deferred, true)
    camera_deferred_y = read_float(camera_deferred + 4, true)
    camera_deferred_z = read_float(camera_deferred + 8, true)
    camera_tgt_x = read_float(camera_deferred + 12, true)
    camera_tgt_y = read_float(camera_deferred + 16, true)
    camera_tgt_z = read_float(camera_deferred + 20, true)

    framebuffer[#framebuffer + 1] = string.format("c %f %f %f %f %f %f %f %f\n", camera_side, camera_deferred_x, camera_deferred_y, camera_deferred_z, camera_tgt_x, camera_tgt_y, camera_tgt_z, camera_yaw)
end

function send_mario()
    framebuffer[#framebuffer + 1] = string.format("m %f %f %f %f\n", mario_x, mario_y, mario_z, mario_rot)
end


-- event loop
while true do
	framebuffer = {}

	mario_x = read_float(mario_x_ptr, true)
	mario_y = read_float(mario_y_ptr, true)
	mario_z = read_float(mario_z_ptr, true)
	mario_rot = read_float(mario_rot_ptr, true)

	send_colliders()
    send_mario()

	sock_client = socket.connect('127.0.0.1', 5617)

	if sock_client ~= nil then
		sock_client:settimeout(nil)

		framebuffer[#framebuffer + 1] = string.format("e %d %s\n", emu.framecount(), useragent)
		sock_client:send(table.concat(framebuffer, ""))

		sock_client:close()
	end

    emu.frameadvance()
end
