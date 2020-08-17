#
# Copyright Contributors to the OpenTimelineIO project
#
# Licensed under the Apache License, Version 2.0 (the "Apache License")
# with the following modification; you may not use this file except in
# compliance with the Apache License and the following modification to it:
# Section 6. Trademarks. is deleted and replaced with:
#
# 6. Trademarks. This License does not grant permission to use the trade
#    names, trademarks, service marks, or product names of the Licensor
#    and its affiliates, except as required to comply with Section 4(c) of
#    the License and to reproduce the content of the NOTICE file.
#
# You may obtain a copy of the Apache License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the Apache License with the above modification is
# distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the Apache License for the specific
# language governing permissions and limitations under the Apache License.
#

import os
import unittest

import opentimelineio as otio
import opentimelineio.test_utils as otio_test_utils

TimeRange = otio.opentime.TimeRange
RationalTime = otio.opentime.RationalTime


SAMPLE_DATA_DIR = os.path.join(os.path.dirname(__file__), "sample_data")
SCREENING_EXAMPLE_PATH = os.path.join(SAMPLE_DATA_DIR, "screening_example.edl")

class BasicEventTests(unittest.TestCase, otio_test_utils.OTIOAssertions):
    """
    Basic event units
    """
    def setUp(self):
        self.track = otio.schema.Track(name="V1")
        self.clip = otio.schema.Clip(name="clip1")
        self.range_ = TimeRange(RationalTime(1, 24), RationalTime(25, 24))

    def test_modify_source_range(self):
        adjust_item = otio.edit.ModifyItemSourceRangeEdit(
            name='set_new_range', item=self.clip, new_range=self.range_
        )
        adjust_item.run()
        self.assertEqual(self.clip.source_range, self.range_)

    def test_insert_item(self):
        insert_item = otio.edit.InsertItemEdit(
            name='insert_yo', item=self.clip, track=self.track
        )
        insert_item.run()
        self.assertEqual(len(self.track), 1)

    def test_remove_item(self):
        self.track.append(self.clip)
        self.assertEqual(self.track[0], self.clip)
        remove = otio.edit.RemoveItemEdit(
            name='remove_yo', item=self.clip
        )
        remove.run()
        self.assertEqual(len(self.track), 0)

    def test_event_stack(self):
        stack = otio.event.EventStack(name='events')

        adjust_item = otio.edit.ModifyItemSourceRangeEdit(
            name='set_new_range', item=self.clip, new_range=self.range_
        )
        stack.add_event(adjust_item)
        insert_item = otio.edit.InsertItemEdit(
            name='foo', index=0, track=self.track, item=self.clip
        )
        stack.add_event(insert_item)

        stack.run()
        self.assertEqual(len(self.track), 1)
        self.assertEqual(self.track[0].source_range, self.range_)



class EditCommandTests(unittest.TestCase, otio_test_utils.OTIOAssertions):
    """
    Test the variable edit events
    """
    def setUp(self):
        self.track = otio.schema.Track(name="V1")
        self.clip = otio.schema.Clip(name="clip1")

    def new_clip(self, name):
        return otio.schema.Clip(name=name, source_range=TimeRange(
            RationalTime(0, 24),
            RationalTime(10, 24)
        ))

    def test_overwrite(self):
        self.clip.source_range = TimeRange(
            RationalTime(1, 24), RationalTime(25, 24)
        )
        self.track.append(self.clip)

        # -- First, we test with placing passed all existing content
        c2 = self.new_clip("passed")
        otio.edit.overwrite(c2, self.track, track_time=RationalTime(40, 24))

        self.assertEqual(len(self.track), 3) # Account for the Gap
