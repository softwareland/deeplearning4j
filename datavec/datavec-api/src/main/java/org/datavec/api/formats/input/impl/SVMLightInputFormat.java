/*******************************************************************************
 * Copyright (c) 2015-2018 Skymind, Inc.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License, Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

package org.datavec.api.formats.input.impl;



import org.datavec.api.conf.Configuration;
import org.datavec.api.formats.input.BaseInputFormat;
import org.datavec.api.records.reader.RecordReader;
import org.datavec.api.records.reader.impl.misc.SVMLightRecordReader;
import org.datavec.api.split.InputSplit;

import java.io.IOException;

/**
 * SVMLight input format
 *
 * @author Adam Gibson
 */
public class SVMLightInputFormat extends BaseInputFormat {
    @Override
    public RecordReader createReader(InputSplit split, Configuration conf) throws IOException, InterruptedException {
        return createReader(split);
    }

    @Override
    public RecordReader createReader(InputSplit split) throws IOException, InterruptedException {
        SVMLightRecordReader reader = new SVMLightRecordReader();
        reader.initialize(split);
        return reader;
    }


}
